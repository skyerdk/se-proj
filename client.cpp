#include <array>
#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <string>
#include <fstream>

using namespace std;
using boost::asio::ip::tcp;

string clint_file_name = "client.txt";
string m_fix = "";
string price, num, res;
void order_fix();
bool _quit = false;
string m_orderid;
string b_o_s;

vector<string> m_split(string str, char pattern) //切分订单
{
	vector<string> result;
	int size = str.size();
	int p = 0;
	for (int i = 0; i < size; i++) {
		if (str[i] == pattern) {
			string mes;
			for (int j = p; j < i; j++)
				mes += str[j];
			p = i + 1;
			result.push_back(mes);
		}
	}
	return result;
}

string order_kind()//确定order类型 
{
	string mes1;
	while (true) 
	{
		cout << "1:creat a new order" << '/n'
			<< "2:cancel order" << '/n'
			<< "0 :quit " << endl;
		cin >> mes1;
		if (mes1 == "1") 
		{
			char mes2;
			m_fix += "35=D;";
			while (true) 
			{
				cout << "3:buy"<<'/n'
					<<"4:sale " <<'/n'
					<<"0:quit"<< endl;
				cin >> mes2;
				switch (mes2) 
				{
				case '3':
				{
					m_fix += "54=1;";
					b_o_s = "Buy";
					break;
				}
				case '4':
				{
					m_fix += "54=2;";
					b_o_s = "Sale";
					break;
				}
				case '0':
				{
					mes1 = "0";
					break;
				}
				default:
					cout << "invalid!" << endl;
				}
			}
			break;
		}
		else if (mes1 == "2") 
		{
			m_fix += "35=F;";
			break;
		}
		else if (mes1 == "0") break;
		else
			cout << "invalid input." << endl;
	}
	return mes1;
}

void order_number() //确定数量
{
	while (!_quit) 
	{
		bool flag = true;
		cout << "Please enter the number(-1 to quit): "
			<< endl;
		cin >> num;
		if (num == "-1") _quit = true;
		else 
		{
			for (int i = 0; i < num.size(); i++) 
			{
				if (num[i] > '9' || num[i] < '0') 
				{
					cout << "invalid input." << endl;
					flag = false;
					break;
				}
			}
		}
		if (flag && !_quit) 
		{
			m_fix += "38=" + num + ";";
			break;
		}
	}
}

void order_price()//确定价格 
{
	while (!_quit) 
	{
		bool judge = true;
		cout << "Please give your price(-1 to quit): " 
			<< endl;
		cin >> price;
		if (price == "-1") _quit = true;
		else 
		{
			int _num = 0;
			for (int i = 0; i < price.size(); i++) {
				if ((price[i] > '9' || price[i] < '0') && price[i] != '.') 
				{
					cout << "invalid input." << endl;
					judge = false;
					break;
				}
				if (price[i] == '.') {
					if (i == 0 || i != price.size() - 3) 
					{
						cout << "invalid input." << endl;
						judge = false;
						break;
					}
					_num++;
				}
			}
			if (judge && _num != 1) 
			{
				cout << "invalid input." << endl;
				judge = false;
			}
		}
		if (judge && !_quit) 
		{
			m_fix += "44=" + price + ";";
			break;
		}
	}
}

int neworder() 
{
	order_price();
	order_number();
	if (_quit) return -1;
	return 0;
}

int cancelorder() //取消订单
{
	string mes;
	cout << "Please enter the orderid(-1 to quit): " 
		<< endl;
	cin >> mes;
	if (mes == "-1")
	{
		return -1;
	}
	m_orderid = mes;
	m_fix += "41=" + mes + ";";
	return 0;
}

void order_fix() //获得fix内容
{
	m_fix.clear();
	_quit = false;
	string mes1;
	int mes2;
	mes1 = order_kind();
	if (mes1 == "-1")
	{
		m_fix = "-1";
		mes2 = 0;
	}
	if (mes1 == "1")
		mes2 = neworder();
	if (mes1 == "2")
		mes2 = cancelorder();
	if (mes1 == "0" || mes2 == -1) 
	{
		order_fix();
	}
}

int main(int argc, char* argv[]) 
{
	string query_ip;
	cout << "please input your ip";
	cin >> query_ip;
	while (1) 
	{
		order_fix();
		if (m_fix == "-1") break;

		boost::asio::io_service io_service;
		tcp::resolver resolver(io_service);
		tcp::resolver::query query(query_ip, "9876");
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		tcp::socket socket(io_service);
		boost::asio::connect(socket, endpoint_iterator);
		boost::system::error_code error;

		boost::asio::write(socket, boost::asio::buffer(m_fix), error);

		array<char, 256> input_buffer;
		size_t rsize = socket.read_some(boost::asio::buffer(input_buffer), error);
		string r_fix = string(input_buffer.data(), input_buffer.data() + rsize);

		vector<string> R_fix = m_split(r_fix, ';');
		vector<string> New_orderid;
		if (R_fix[0] == "35=8") 
		{
			string new_orderid = R_fix[3];
			new_orderid += "=";
			New_orderid = m_split(new_orderid, '=');

			if (R_fix[1] == "150=2" && R_fix[2] == "39=2") 
			{
				cout << b_o_s << '/n' << "Price: " << price << '/n'
					<< "Number: " << num << '/n' << "Full fill." << '/n'
					<< "Your orderid: " << New_orderid[1] << endl;
				res = "full fill";
			}
			else if (R_fix[1] == "150=1" && R_fix[2] == "39=1") 
			{
				cout << b_o_s << '/n' << "Price: " << price << '/n'
					<< "Number: " << num << '/n'
					<< "Partial fill." << '/n'
					<< "Your orderid: " << New_orderid[1] << endl;
				res = "partial fill";
			}
			else if (R_fix[1] == "150=4" && R_fix[2] == "39=4") 
			{
				cout << "Canceled." << endl;
				res = "cancel successful";
			}
		}
		else 
		{
			cout << "Your cancel request is rejected." << endl;
			res = "reject";
		}

		fstream Clint(clint_file_name, ios::app);
		if (res != "reject") 
		{
			if (res != "cancel successful")
				Clint << b_o_s << "\t" 
				<< "Price: " << price << "\t" 
				<< "Number: " << num << "\t" 
				<< "Orderid: " << New_orderid[1] << "\t" 
				<< res << endl;
			else
				Clint << "Order " << m_orderid << " is canceled.";
		}

		Clint.close();
	}
	return 0;
}

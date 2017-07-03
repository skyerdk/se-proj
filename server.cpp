#include <array>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <time.h>
#include "stdlib.h"
#include <boost/asio.hpp>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
using boost::asio::ip::tcp;
using std::chrono::system_clock;
using namespace std;

string OB_name = "orderbook.txt";
string Orderids_name = "Orderids.txt";
string m_fix = "";

vector<vector<string>> Orders;
vector<string> orders;


string set_orderid() //抓取系统时间作为orderid
{
	auto now = system_clock::now();
	auto now_c = system_clock::to_time_t(now);
	ostringstream ss;
	ss << put_time(localtime(&now_c), "%Y%m%d%H%M");
	string orderid = ss.str();
	return orderid;
}



vector<string> m_split(string str, char pattern) //划分订单信息
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

void create_vector(string file_name) //创建订单容器
{
	ifstream Orders_file(file_name);
	while (!Orders_file.eof()) {
		string line;
		getline(Orders_file, line);
		if (line != "") {
			vector<string> m_order = m_split(line, ';');
			for (int i = 0; i < m_order.size(); i++) {
				string mes = m_order[i];
				orders.push_back(mes);
			}
			Orders.push_back(orders);
			orders.clear();
		}
	}
	Orders_file.close();
	ofstream Orders_file_c(file_name);
	Orders_file_c.clear();
	Orders_file_c.close();
}



bool new_order(string price, string number, string buy_or_sale) //新建订单
{
	if (buy_or_sale == "3")return false;
	create_vector(OB_name);
	int ordersnum = Orders.size();
	bool find = false;
	int a;
	stringstream ss;
	int num, g_num, new_num;
	ss << number;
	ss >> g_num;
	ss.clear();
	new_num = g_num;
	for (int i = 0; i < ordersnum; i++) 
	{
		if (Orders[i][0] == price) 
		{
			find = true;
			string NUM = Orders[i][1];
			ss << NUM;
			ss >> num;
			ss.clear();
			if (buy_or_sale == "1")
				new_num = num - g_num;
			if (buy_or_sale == "2")
				new_num = num + g_num;
			a = i;
			break;
		}
	}
	ss << new_num;
	string m_number;
	m_number = ss.str();
	ss.clear();
	m_fix = m_fix + "151=" + m_number + ";";
	int m_flag;
	if (find) 
	{
		Orders[a][1] = m_number;
		if ((buy_or_sale == "1" && num < 0) || (buy_or_sale == "2" && num > 0))
			m_flag = 1;
		else m_flag = 2;
	}
	if (!find) 
	{
		Orders.push_back({ price,m_number });
		m_flag = 0;
	}
	fstream Orders_file_w(OB_name);
	int o_size = Orders.size();
	for (int i = 0; i < o_size; i++) 
	{
		for (int j = 0; j < 2; j++)
			Orders_file_w << Orders[i][j] << ";";
		Orders_file_w << endl;
	}
	Orders_file_w.close();
	Orders.clear();
	if (m_flag == 0 || m_flag == 1)
		return false;
	return true;
}

bool Cancel(string Given_orderid) //取消订单
{
	Given_orderid += "=";
	string given_orderid = m_split(Given_orderid, '=')[1];
	create_vector(Orderids_name);
	int ordersnum = Orders.size();
	bool find = false;
	int a;
	string Kind, Price, Number;
	for (int i = 0; i < ordersnum; i++) 
	{
		if (Orders[i][0] == given_orderid) 
		{
			find = true;
			Kind = Orders[i][1];
			Price = Orders[i][2];
			Number = Orders[i][3];
			a = i;
			break;
		}
	}
	if (find) 
	{
		if (Orders[a][1] == "3")
			find = false;
		Orders[a][1] = "3";
	}
	fstream Orders_file_w(Orderids_name);
	for (int i = 0; i < ordersnum; i++) 
	{
		for (int j = 0; j < 4; j++)
			Orders_file_w << Orders[i][j] << ";";
		Orders_file_w << endl;
	}
	Orders_file_w.close();
	Orders.clear();
	if (find)new_order(Price, Number, Kind);
	if (!find) 
	{
		return false;
	}
	return true;
}

int main() {
	boost::asio::io_service io_service;
	tcp::acceptor acc(io_service, tcp::endpoint(tcp::v6(), 9876));
	string query_ip;
	cout << "please input query ip" << endl;
	cin >> query_ip;
	while (1) 
	{
		srand(time(NULL));
		boost::system::error_code ignored;

		tcp::socket socket(io_service);
		acc.accept(socket);

		array<char, 256> input_buffer;
		size_t input_size = socket.read_some(boost::asio::buffer(input_buffer), ignored);
		string r_fix(input_buffer.data(), input_buffer.data() + input_size);

		vector<string> R_fix = m_split(r_fix, ';');

		if (R_fix[0] == "35=D") 
		{
			string message;
			string giv_price = R_fix[2];
			string giv_number = R_fix[3];
			giv_price += '=';
			giv_number += '=';
			vector<string> Price = m_split(giv_price, '=');
			vector<string> Number = m_split(giv_number, '=');
			string price = Price[1];
			string number = Number[1];
			m_fix = m_fix + "31=" + price + ";32=" + number + ";";

			string kind;
			bool full;
			if (R_fix[1] == "54=1") 
			{
				full = new_order(price, number, "1");
				kind = "1";
			}
			if (R_fix[1] == "54=2") 
			{
				full = new_order(price, number, "2");
				kind = "2";
			}
			if (full)message = "35=8;150=2;39=2;";
			if (!full)message = "35=8;150=1;39=1;";

			string m_orderid;
			m_orderid = set_orderid();
			message = message + "11=" + m_orderid + ";";

			fstream Orderfile(Orderids_name, ios::app);
			Orderfile << m_orderid << ";" << kind << ";" << price << ";" << number << ";" << endl;
			Orderfile.close();

			boost::asio::write(socket, boost::asio::buffer(message), ignored);

			socket.shutdown(tcp::socket::shutdown_both, ignored);
			socket.close();

			//未完成的hw3
			/*boost::asio::io_service io_service;
			tcp::resolver resolver(io_service);
			tcp::resolver::query query(query_ip, "9876");
			tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
			tcp::socket socket(io_service);
			boost::asio::connect(socket, endpoint_iterator);
			boost::system::error_code error;

			boost::asio::write(socket, boost::asio::buffer(m_fix), error);*/
		}
		if (R_fix[0] == "35=F") 
		{

			bool find = Cancel(R_fix[1]);
			string message;

			if (find) 
			{
				message = "35=8;150=4;39=4;";
				string m_orderid;
				m_orderid = set_orderid();
				message = message + "11=" + m_orderid + ";";
			}
			if (!find)message = "35=9;39=8;";

			boost::asio::write(socket, boost::asio::buffer(message), ignored);

			socket.shutdown(tcp::socket::shutdown_both, ignored);
			socket.close();
		}
	}
	return 0;
}

#include <iostream>
#include <string>
#include <unordered_map>
#include <conio.h>
#include <fstream>
#include <ostream>
#include <map>
#include <Windows.h>
#include <chrono>
#include <thread>

std::vector<std::string> split_string(char delim, std::string split_string);
unsigned int load_file(std::string& pass_out, std::map<std::string, std::string>& pwd_map);
bool save_file(std::string password, std::map<std::string, std::string> pwd_map);
std::string encrypt_decrypt(std::string key, std::string encode);
void copy_to_clipboard(std::string string);
std::string prompt_password(bool verify);

namespace
{
	const std::string DEFAULT_FILENAME = "passxor_db";
	const std::string TEST_LOADFILE = "passxor";
}

void show_menu()
{
	std::cout << "Main Menu - Commands" << std::endl;
	std::cout << "save   - Outputs (and overrwrites) the database file" << std::endl;
	std::cout << "get    - Copies a database key's value (password) to the clipboard" << std::endl;
	std::cout << "set    - Enters a new key pair into the database" << std::endl;
	std::cout << "print  - Prints all the keys that exist in the database" << std::endl;
	std::cout << "reset  - Resets the master password" << std::endl;
	std::cout << "menu   - Shows this menu" << std::endl;
	std::cout << "exit   - Exits the program (does NOT save)" << std::endl;
}

int main()
{	
	std::cout << "Welcome to passxor!" << std::endl;
	std::cout << "Written by: Aleksander Krimsky" << std::endl;
	std::cout << "Written on: 28 December 2019" << std::endl;
	std::cout << std::endl;
	std::map<std::string, std::string> pwd_map;
	unsigned int attempts = 0;

load_program:
	std::string master_pwd;
	unsigned int load_status = load_file(master_pwd, pwd_map);
	if (load_status == 2)
	{
		std::cout << "Invalid password..." << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(++attempts));
		goto load_program;
	}
	else if (load_status == 1)
	{
		std::cout << "Create initial master password" << std::endl;
		master_pwd = prompt_password(true);
		std::cout << "Created master password!" << std::endl;
	}
	else
	{
		std::cout << "Created database loaded!" << std::endl;
	}

	show_menu();

main_menu:
	std::cout << "Enter Command: ";
	std::string command;
	std::cin >> command;

	if (command.compare("exit") == 0)
	{
		return 0;
	}
	else if (command.compare("menu") == 0)
	{
		show_menu();
		goto main_menu;
	}
	else if (command.compare("print") == 0)
	{
		for (auto it = pwd_map.begin(); it != pwd_map.end(); it++)
		{
			std::cout << it->first << std::endl;
		}
		goto main_menu;
	}
	else if (command.compare("save") == 0)
	{
		bool result = save_file(master_pwd, pwd_map);
		std::string sresult = "Save successful: ";
		sresult += result ? "true" : "false";
		std::cout << sresult << std::endl;
		goto main_menu;
	}
	else if (command.compare("get") == 0)
	{
		std::cout << "(Get) Enter Key: ";
		std::string login_key;
		std::cin >> login_key;
		if (pwd_map.count(login_key) == 0)
		{
			std::cout << ("Key <" + login_key + "> does not exist") << std::endl;
			goto main_menu;
		}
		else
		{
			copy_to_clipboard(pwd_map[login_key]);
			std::cout << ("Password <" + login_key + "> copied to clipboard") << std::endl;
			goto main_menu;
		}
	}
	else if (command.compare("set") == 0)
	{
		std::cout << "(Set) Enter Login Key: ";
		std::string login_key;
		std::cin >> login_key;
		if (pwd_map.count(login_key))
		{
			std::cout << "Key <" << login_key << "> already exists, do you want to reset?: ";
			std::string check;
			std::cin >> check;
			if (check.compare("y") != 0 && check.compare("yes") == 0)
			{
				goto main_menu;
			}			
		}
		pwd_map[login_key] = prompt_password(true);
		std::cout << ("Password has been set for: " + login_key) << std::endl;
		goto main_menu;
	}
	else if (command.compare("reset") == 0)
	{
		std::cout << "Are you sure you want to reset?: ";
		std::string check;
		std::cin >> check;
		if (check.compare("y") == 0 || check.compare("yes") == 0)
		{
			master_pwd = prompt_password(true);
			std::cout << ("Master password has been reset!") << std::endl;
		}
		goto main_menu;
	}
	else
	{
		std::cout << ("Invalid command: " + command) << std::endl;
		goto main_menu;
	}
	return 0;
}

unsigned int load_file(std::string& pass_out, std::map<std::string, std::string>& pwd_map)
{
	std::ifstream f_stream(DEFAULT_FILENAME, std::ifstream::binary);
	if (!f_stream.good())
	{
		return 1;
	}

	std::cout << "Database file exists. Prompting for password..." << std::endl;
	std::string password = prompt_password(false);
	std::string content;
	content.assign(
		(std::istreambuf_iterator<char>(f_stream)),
		(std::istreambuf_iterator<char>()));

	f_stream.close();
	std::string buffer = encrypt_decrypt(password, content);
	std::string scompare = buffer.substr(0, TEST_LOADFILE.size());
	if (TEST_LOADFILE.compare(scompare))
	{
		return 2;
	}
	buffer.erase(buffer.begin(), buffer.begin() + TEST_LOADFILE.length());

	std::vector<std::string> enc_pairs = split_string(':', buffer);
	for (std::string decrypted : enc_pairs)
	{
		std::vector<std::string> pair = split_string(',', decrypted);
		pwd_map[pair[0]] = pair[1];
	}
	pass_out = password;
	return 0;
}

bool save_file(std::string password, std::map<std::string, std::string> pwd_map)
{
	std::ofstream f_stream(DEFAULT_FILENAME, std::ofstream::binary);
	if (f_stream.is_open())
	{
		std::string complete = TEST_LOADFILE;
		for (auto it = pwd_map.begin(); it != pwd_map.end(); it++)
		{
			std::string pair_key = it->first + "," + it->second;
			if (std::next(it) != pwd_map.end())
				pair_key += ":";
			complete += pair_key;
		}
		std::string encrypted = encrypt_decrypt(password, complete);
		f_stream.write(encrypted.c_str(), encrypted.size());
		f_stream.close();
		return true;
	}
	return false;
}

std::string prompt_password(bool verify)
{
main_entry:
	std::cout << "Enter password: ";
	std::string pass = "";
	char ch;
	ch = _getch();
	while (ch != VK_RETURN)
	{
		if (ch == VK_BACK)
		{
			if (!pass.empty())
			{
				pass.pop_back();
			}
		}
		else if (std::isprint(ch))
		{
			pass.push_back(ch);			
		}
		//system("cls");
		printf("\33[2K\r");
		std::cout << "Enter password: ";
		for (size_t i = 0; i < pass.length(); i++)
		{
			std::cout << "*";
		}
		ch = _getch();
	}
	std::cout << std::endl;
	if (verify)
	{
		bool verified = false;
		std::cout << "For verification, please re-enter the password" << std::endl;
		unsigned int attempts = 0;
		while (!verified)
		{
			std::string test = prompt_password(false);
			if (test.compare(pass))
			{
				std::cout << "First password does not match second password, try again" << std::endl;
				std::cout << "Attempt " << ++attempts << "/3" << std::endl;
				if (attempts > 2)
				{
					std::cout << "Failed to enter password correctly twice, enter a new password" << std::endl;
					goto main_entry; //Don't call the function recursively
				}
			}
			else
			{
				verified = true;
			}
		}
	}
	return pass;
}

std::string encrypt_decrypt(std::string key, std::string encode)
{
	std::string output = encode; //to match length
	for (size_t i = 0; i < encode.length(); i++)
	{
		output[i] = encode[i] ^ key[i % key.length()];
	}
	return output;
}

std::vector<std::string> split_string(char delim, std::string split_string)
{
	std::vector<std::string> vector;
	std::string splitter(split_string);
	while (splitter.find(delim) != std::string::npos)
	{
		auto sfind = splitter.find(delim);
		vector.push_back(splitter.substr(0, sfind));
		splitter.erase(0, sfind + 1);
	}
	vector.push_back(splitter);
	return vector;
}

void copy_to_clipboard(std::string string)
{
	HGLOBAL glob = GlobalAlloc(GMEM_FIXED, string.size() + 1);
	memcpy(glob, string.c_str(), string.size());

	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, glob);
	CloseClipboard();
}
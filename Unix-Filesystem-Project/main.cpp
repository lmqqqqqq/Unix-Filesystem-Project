#define _CRT_SECURE_NO_WORNINGS
#include "Kernel.h"
#include "Utility.h"

int main()
{
	Kernel* k = Kernel::getInstance();
	k->initialize();
	

	while (true)
	{
		cout << "[UNIX-FileSystem " << k->curdir << " ] $";
		char input[100];
		cin.getline(input, 100);
		vector<char*> result = Utility::parseCmd(input);
		if (result.size() > 0)
		{
			if (strcmp(result[0], "help") ==0)
			{
				cout << "----------------------UNIX-FileSystem-----------------------" << endl;
				cout << "[Commands provided]:" << endl;
				cout << "fformat                                : 格式化文件卷" << endl;
				cout << "ls                                     : 列目录" << endl;
				cout << "mkdir <dirname>                        : 创建目录" << endl;
				cout << "fcreate <filename>                     : 新建文件" << endl;
				cout << "fopen <filename>                       : 打开文件" << endl;
				cout << "fclose <fd>                            : 关闭文件" << endl;
				cout << "fread <fd> <nbytes>                    : 读文件" << endl;
				cout << "fwrite <fd> <nbytes> <string>          : 写文件" << endl;
				cout << "flseek <fd> <offset> <ptrname>         : 定位文件读写指针" << endl;
				cout << "fdelete <filename>                     : 删除文件" << endl;
				cout << "cd <dirname>                           : 改变当前目录" << endl;
				cout << "cp <file1> <file2>                     : 拷贝文件" << endl;
				cout << "frename <file1> <file2>                : 重命名文件" << endl;
				cout << "ftree <dirname>                        : 显示目录树" << endl;
				cout << "pwd                                    : 显示当前目录" << endl;
				cout << "exit                                   : 退出" << endl;
				cout << "----------------------UNIX-FileSystem-----------------------" << endl;
				cout << endl;
			}
			else if (strcmp(result[0], "cd") == 0)
			{
				if (result.size() > 1)
				{
					if(strcmp(result[1], "..") ==0)
					{
						char* padir = k->curdir;
						if(padir[1]=='\0')
						{
							cout << "Already in the root" << endl;
						}
						else
						{
							for (int i = 127; i >= 0; i--)
							{
								if (padir[i] != '/')
								{
									padir[i] = '\0';
								}
								else
								{
									if(i!=0)
									{
										padir[i] = '\0';
									}
									break;
								}
							}
							k->cd(padir);
							//cout << padir;
							if (k->error == Kernel::NOTDIR)
								cout << padir << ": Not a directory" << endl;
							else if (k->error == Kernel::NOENT)
								cout << padir << ": No such file or directory" << endl;
						}
						
					}
					else
					{
						k->cd(result[1]);
						if (k->error == Kernel::NOTDIR)
							cout << result[1] << ": Not a directory" << endl;
						else if (k->error == Kernel::NOENT)
							cout << result[1] << ": No such file or directory" << endl;
					}
					
				}
				else
					cout << "Operand missed" << endl;
			}
			else if (strcmp(result[0], "fformat") == 0)
			{
				k->format();
				k->initialize();
			}
			else if (strcmp(result[0], "mkdir") == 0)
			{
				if (result.size() > 1)
				{
					k->mkdir(result[1]);
					if (k->error == Kernel::NOENT)
						cout << "No such a file or directory" << endl;
					if (k->error == Kernel::ISDIR)
						cout << result[1] << ": This is a directory" << endl;
				}
				else
					cout << "Operand missed" << endl;

			}
			else if (strcmp(result[0], "ls") == 0)
			{
				k->ls();
			}
			else if (strcmp(result[0], "fopen") == 0)
			{
				if (result.size() > 1)
				{
					const int fd = k->open(result[1], 511);
					if (k->error == Kernel::NO_ERROR)
						cout << "fd = " << fd << endl;
					else if (k->error == Kernel::ISDIR)
						cout << result[1] << ": This is a directory" << endl;
					else if (k->error == Kernel::NOENT)
						cout << result[1] << ": No such a file or directory" << endl;
				}
				else
					cout << "Operand missed" << endl;

			}
			else if (strcmp(result[0], "fcreate") == 0)
			{
				int fd;
				if (result.size() > 1)
				{
					fd = k->create(result[1], 511);

					if (k->error == Kernel::NOENT)
						cout << "No such a file or directory" << endl;
					if (k->error == Kernel::ISDIR)
						cout << result[1] << ": This is a directory" << endl;
					if (k->error == Kernel::NO_ERROR)
						cout << "fd = " << fd << endl;
				}
				else
					cout << "Operand missed" << endl;
			}
			else if (strcmp(result[0], "fclose") == 0)
			{
				if (result.size() > 1)
					k->close(atoi(result[1]));
				else
					cout << "Operand missed" << endl;
			}
			else if (strcmp(result[0], "fread") == 0)
			{
				int actual;
				if (result.size() > 2) {
					char* buf;
					buf = new char[atoi(result[2])];
					buf[0] = '\0';
					actual = k->fread(atoi(result[1]), buf, atoi(result[2]));
					if (actual == -1)
					{
						if (k->error == Kernel::BADF)
							cout << atoi(result[1]) << ":This fd is wrong" << endl;
					}
					else
					{
						if (actual > 0)
						{
							for (int i = 0; i < actual; i++)
							{
								cout << buf[i];
							}
							cout << endl;
						}
						cout << "Successfully read " << actual << " bytes" << endl;
					}
					delete buf;
				}
				else
					cout << "Operand missed" << endl;
			}
			else if (strcmp(result[0], "fwrite") == 0)
			{
				int actual;
				if (result.size() > 3)
				{
					if (atoi(result[2]) > strlen(result[3]))
					{
						cout << "Read nbytes can not be larger than the length of the string" << endl;
						continue;
					}
					actual = k->fwrite(atoi(result[1]), result[3], atoi(result[2]));
					if (actual == -1)
					{
						if (k->error == Kernel::BADF)
							cout << atoi(result[1]) << ": This fd is wrong" << endl;
					}
					else
					{
						cout << "Successfully write " << actual << " bytes" << endl;
					}
				}
				else
				{
					cout << "Operand missed" << endl;
				}
			}
			else if (strcmp(result[0], "flseek") == 0)
			{
				if (result.size() > 3)
				{
					if (atoi(result[3]) >= 0 && atoi(result[3]) <= 5)
					{
						k->fseek(atoi(result[1]), atoi(result[2]), atoi(result[3]));
						if (k->error == Kernel::BADF)
							cout << atoi(result[1]) << ": This fd is wrong" << endl;
					}
					else
						cout << result[3] << ": This ptrname is wrong" << endl;

				}
				else
				{
					cout << "Operand missed" << endl;
				}
			}
			else if (strcmp(result[0], "fdelete") == 0)
			{
				if (result.size() > 1)
				{
					k->fdelete(result[1]);
					if (k->error == Kernel::NOENT)
						cout << result[1] << ": No such a file or directory" << endl;
				}
				else
				{
					cout << "Operand missed" << endl;
				}
			}
			else if (strcmp(result[0], "cp") == 0)
			{
				if (result.size() > 2)
				{
					k->fmount(result[1], result[2]);
					if (k->error == Kernel::NOENT)
						cout << result[2] << ": No such a file or directory" << endl;
					else if (k->error == Kernel::NOOUTENT)
						cout << result[1] << ": No such a file or directory" << endl;
					else if (k->error == Kernel::ISDIR)
						cout << result[2] << ": This is a directory" << endl;
				}
				else
				{
					cout << "Operand missed" << endl;
				}
			}
			else if (strcmp(result[0], "frename") == 0)
			{
				if (result.size() > 2)
				{
					k->frename(result[1], result[2]);
				}
				else
				{
					cout << "Operand missed" << endl;
				}
			}
			else if (strcmp(result[0], "ftree") == 0)
			{
				if (result.size() > 1)
				{
					string path = result[1];
					k->ftree(path);
				}
				else
				{
					cout << "Operand missed" << endl;
				}
			}
			else if (strcmp(result[0], "exit") == 0)
			{
				k->clear();
				break;
			}
			else if (strcmp(result[0], "pwd") == 0)
			{
				cout << k->curdir << endl;
			}
			else {
				cout << "command \'" << result[0] << "\' not found" << endl;
			}
		}
	}
	return 0;
}
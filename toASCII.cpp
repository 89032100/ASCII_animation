#include <iostream>
#include <string>
#include <windows.h>
#include <fstream>
using namespace std;
typedef struct
{
	BYTE b;
	BYTE g;
	BYTE r;
}RGB;
RGB& operator*(RGB& a, float b) {
	a.b *= b;
	a.g *= b;
	a.r *= b;
	return a;
}
RGB& operator+(RGB& a, RGB& b) {
	a.b += b.b;
	a.g += b.g;
	a.r += b.r;
	return a;
}
class ASCII {
public:
	RGB word[8][8];
};
ASCII Map[95]; //存所有字元的資料
class BMP {
public:
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;
	ifstream pfin;
	int offset;
	RGB** img;
	string output = "";
	void load(const char* path) { //讀取
		pfin.open(path, ios::binary);
		pfin.read((char*)&fileHeader, sizeof(BITMAPFILEHEADER));
		pfin.read((char*)&infoHeader, sizeof(BITMAPINFOHEADER));
		offset = (infoHeader.biWidth * 3) % 4;
		if (offset != 0) {
			offset = 4 - offset;
		}
		img = new RGB * [infoHeader.biHeight];
		for (int i = infoHeader.biHeight - 1; i >= 0; i--) {
			img[i] = new RGB[infoHeader.biWidth];
			for (int j = 0; j < infoHeader.biWidth; j++) {
				pfin.read((char*)(&img[i][j]), sizeof(RGB));
			}
			if (offset != 0) {
				char ignore;
				for (int k = 0; k < offset; k++) {
					pfin.read(&ignore, sizeof(char));
				}
			}
		}
		pfin.close();
	}
	void loadmap() { //讀ASCII字元
		for (int i = 0; i < 95; i++) {
			for (int j = 0; j < 8; j++) {
				for (int k = i * 8; k < i * 8 + 8; k++) {
					Map[i].word[j][k % 8] = img[j][k];
				}
			}
		}
	}
	void ToGray() { //灰階+二值化
		for (int i = 0; i < infoHeader.biHeight; i++) {
			for (int j = 0; j < infoHeader.biWidth; j++) {
				int pixel = (img[i][j].r * 299 + img[i][j].g * 587 + img[i][j].b * 114 + 500) / 1000;
				int temp = 78;
				img[i][j].r = pixel < temp ? 255 : 0;
				img[i][j].g = pixel < temp ? 255 : 0;
				img[i][j].b = pixel < temp ? 255 : 0;
			}
		}
	}
	void ToASCII() { //轉ASCII
		for (int i = 0; i < infoHeader.biHeight / 8; i++) {
			for (int j = 0; j < infoHeader.biWidth / 8; j++) { //因為字元是8*8所以圖片也分成8*8的區塊
				int compare[95] = { 0 };
				int temp = 0;
				for (int n = 0; n < 95; n++) {
					for (int y = i * 8; y < i * 8 + 8; y++) {
						for (int x = j * 8; x < j * 8 + 8; x++) { //把每個區塊拿去和所有字元比較
							if (Map[n].word[y % 8][x % 8].b == img[y][x].b) {
								compare[n]++;
							}
						}
					}
					if (compare[n] >= 64) { //如果這個字元已經是最像的就跳出
						temp = n;
						break;
					}
					if (compare[temp] < compare[n]) { //比較所有字元哪個字元最像
						temp = n;
					}
				}
				output += char(32 + temp);
			}
			output += '\n';
		}
	}
	void resize(float size) { //圖片縮放 雙線性插值
		RGB** Newimg;
		int NewWidth = int(infoHeader.biWidth * size + 0.5);
		int NewHeight = int(infoHeader.biHeight * size + 0.5);
		Newimg = new RGB * [NewHeight];
		for (int i = 0; i < NewHeight; i++) {
			Newimg[i] = new RGB[NewWidth];
		}
		for (int i = 0; i < NewHeight; i++) {
			for (int j = 0; j < NewWidth; j++) {
				int io = (int)(i / size + 0.5);
				int jo = (int)(j / size + 0.5);
				if ((jo >= 0) && (jo < infoHeader.biWidth) && (io >= 0) && (io < infoHeader.biHeight)) {
					float m = (float)(i / size + 0.5) - io;
					float n = (float)(j / size + 0.5) - jo;
					RGB x1, x2, y1, y2;
					x1 = img[io][jo];
					x2 = jo + 1 >= infoHeader.biWidth ? x1 : img[io][jo + 1];
					y1 = io + 1 >= infoHeader.biHeight ? x1 : img[io + 1][jo];
					y2 = io + 1 >= infoHeader.biHeight ? x2 : img[io + 1][jo + 1];
					Newimg[i][j] = x1 * (1 - m) * (1 - n) + x2 * (1 - m) * n + y1 * m * (1 - n) + y2 * m * n;
				}
				else {
					Newimg[i][j].r = 255;
					Newimg[i][j].g = 255;
					Newimg[i][j].b = 255;
				}
			}
		}
		for (int i = 0; i < infoHeader.biHeight; i++) {
			delete[]img[i];
		}
		img = Newimg;
		infoHeader.biHeight = NewHeight;
		infoHeader.biWidth = NewWidth;
	}
	~BMP() {
		for (int i = 0; i < infoHeader.biHeight; i++) {
			delete[]img[i];
		}
		delete[]img;
	}
};
class App {
public:
	void run() {
		BMP word;
		word.load("ASCII.bmp");
		word.loadmap();
		BMP animation;
		string output[61]; //幾張圖
		for (int i = 0; i < 61; i++) { //幾張圖
			string path = "yzu1072041_" + to_string(i + 1) + ".bmp";
			const char* path_c = path.c_str();
			animation.load(path_c);
			animation.resize(1.88);
			animation.ToGray();
			animation.ToASCII();
			output[i] = animation.output;
			animation.output = "";
			cout << "finished load：" << i + 1 << endl;
		}
		system("cls");
		for (auto& u : output) {
			cout << u;
			Sleep(57);//顯示間隔
			SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ 0,0 });
		}
		system("cls");
	}
};
int main() {
	App app;
	app.run();
	return 0;
}
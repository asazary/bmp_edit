#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cctype>

using namespace std;

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long

#pragma pack(push, 1)
struct _BITMAPFILEHEADER
{
	WORD Type; 				// 'BM' 0x4D42
	DWORD Size; 			// ������ ����� � ������, BitCount*Height*Width+ OffsetBits
	WORD Reserved1; 	// ��������������; ������ ���� ����
	WORD Reserved2; 	// ��������������; ������ ���� ����
	DWORD OffsetBits; // �������� ������ �� ������ ����� � ������
										// = sizeof(_BITMAPFILEHEADER)+sizeof(_BITMAPINFOHEADER)
};
#pragma pack(pop)

struct _BITMAPINFOHEADER
{
	DWORD Size; 					// ����� ������ ����������� ��� ��������� = 40
	DWORD Width; 					// ������ ��������� ������� � ��������
	DWORD Height; 				// ������ ��������� ������� � ��������
	WORD Planes; 					// ����� ���������� �������� ���������� = 1
	WORD BitCount;				// ������� �����, ����� ��� �� ����� = 0,1,4,8,16,24,32
	DWORD Compression;		// ��� ������ = 0 ��� ��������� �����������
	DWORD SizeImage; 			// ������ ����������� � ������ BitCount*Height*Width
	DWORD XPelsPerMeter; 	// ����������� ����������� �� �����������
	DWORD YPelsPerMeter; 	// ����������� ����������� �� ���������
	DWORD ColorUsed; 			// ����� �������� ������������ ������. ���� ��� ����� = 0
	DWORD ColorImportant; // ����� ����������� ������ = 0
};

struct _RGBQUAD;     // ���������, ����� �� ���� ������� � ����������� �����

struct _RGBTRIPLE
{
	BYTE Blue;
	BYTE Green;
	BYTE Red;

	_RGBTRIPLE& operator= (const _RGBQUAD& source);
};

struct _RGBQUAD
{
  BYTE Blue;
  BYTE Green;
  BYTE Red;
  BYTE Reserved;

  _RGBQUAD& operator= (const _RGBQUAD& source);
  _RGBQUAD& operator= (const _RGBTRIPLE& source);
};

// ����������� �������� ������������, �.�. ������ ����� ��� ��� ���������
_RGBTRIPLE& _RGBTRIPLE::operator= (const _RGBQUAD& source)
{
  this->Blue = source.Blue;
  this->Green = source.Green;
  this->Red = source.Red;
  return *this;
}


_RGBQUAD& _RGBQUAD::operator= (const _RGBQUAD& source)
{
  this->Blue = source.Blue;
  this->Green = source.Green;
  this->Red = source.Red;
  this->Reserved = source.Reserved;
  return *this;
}

_RGBQUAD& _RGBQUAD::operator= (const _RGBTRIPLE& source)
{
  this->Blue = source.Blue;
  this->Green = source.Green;
  this->Red = source.Red;
  this->Reserved = 0;
  return *this;
}


// ����������� �������� ��������� ��� ������ ���������� ��������

bool operator == (const _RGBTRIPLE &rgbtriple1, const _RGBTRIPLE &rgbtriple2) {
    return rgbtriple1.Blue == rgbtriple2.Blue && rgbtriple1.Green == rgbtriple2.Green &&
        rgbtriple1.Red == rgbtriple2.Red;
}

bool operator == (const _RGBTRIPLE &rgbtriple, const _RGBQUAD &rgbquad) {
    return rgbtriple.Blue == rgbquad.Blue && rgbtriple.Green == rgbquad.Green &&
        rgbtriple.Red == rgbquad.Red;
}

bool operator == (const _RGBQUAD &rgbquad, const _RGBTRIPLE &rgbtriple) {
    return rgbtriple.Blue == rgbquad.Blue && rgbtriple.Green == rgbquad.Green &&
        rgbtriple.Red == rgbquad.Red;
}

bool operator == (const _RGBQUAD &rgbquad1, const _RGBQUAD &rgbquad2) {
    return rgbquad1.Blue == rgbquad2.Blue && rgbquad1.Green == rgbquad2.Green &&
        rgbquad1.Red == rgbquad2.Red;
}
// --------------------------------------


// ������� �������� ������� ���� � ������� ��� ���������� �����
WORD getColorInPalette(BYTE r, BYTE g, BYTE b, _RGBQUAD *Palette, DWORD colorCnt)
{
  double minDistance = -1, distance;
  WORD num = 0;

  for(int i = 0; i < colorCnt; i++)   // �������� �� ���� �������
  {
      // ��������� ��������� ��� �������� �����
    distance = sqrt(pow(r - Palette[i].Red, 2)
                + pow(g - Palette[i].Green, 2)
                + pow(b - Palette[i].Blue, 2) );

    if(minDistance < 0 || distance < minDistance)
    {
      minDistance = distance;
      num = i;
    }
  }
  return num;   // ���������� ����� �������� �������� ����� � ����������
}

// ������ �� �� �����, �� ����� ������ ������� � �������� ��������
_RGBQUAD getColorInPalette(const _RGBTRIPLE sourcePixel, _RGBQUAD *Palette, DWORD BitCount)
{
  double minDistance = -1, distance;
  WORD num = 0;

  for(int i = 0; i < (int)pow(2, BitCount); i++)
  {
    distance = sqrt(pow(sourcePixel.Red - Palette[i].Red, 2)
                + pow(sourcePixel.Green - Palette[i].Green, 2)
                + pow(sourcePixel.Blue - Palette[i].Blue, 2) );

    if(minDistance < 0 || distance < minDistance)
    {
      minDistance = distance;
      num = i;
    }
  }
  return Palette[num];
}


// ------------------

// ����������� 16-������� �������
const BYTE stdPalette16[16][3] = {
	  {0x00, 0x00, 0x00},
	  {0x80, 0x00, 0x00},
	  {0x00, 0x80, 0x00},
	  {0x80, 0x80, 0x00},
	  {0x00, 0x00, 0x80},
	  {0x80, 0x00, 0x80},
	  {0x00, 0x80, 0x80},
	  {0xC0, 0xC0, 0xC0},
	  {0x80, 0x80, 0x80},
	  {0xFF, 0x00, 0x00},
	  {0x00, 0xFF, 0x00},
	  {0xFF, 0xFF, 0x00},
	  {0x00, 0x00, 0xFF},
	  {0xFF, 0x00, 0xFF},
	  {0x00, 0xFF, 0xFF},
	  {0xFF, 0xFF, 0xFF}
};

// 2-�������� �������
const BYTE stdPalette2[2][3] = {
	{0x00, 0x00, 0x00},
	{0xFF, 0xFF, 0xFF}
};

// ���������� ������� � ����������� �� �������
void fillPalette(_RGBQUAD *Palette, DWORD BitCount)
{
  WORD a;
  if(BitCount == 8)   // 256 ������
  {
    // "����������" �������
    for (WORD i = 0; i < 256; i += 51)
      for(WORD j = 0; j < 256; j += 51)
        for(WORD k = 0; k < 256; k += 51)
        {
          Palette[a].Red = i;
          Palette[a].Green = j;
          Palette[a].Blue = k;
          Palette[a].Reserved = 0;
          a++;
        }

    // � ����� ������� ��� 18 �������� ������
    a = 238;
    for(WORD i = 4; i < 256 && a < 256; i += 14)
    {
      Palette[a].Red = Palette[a].Green = Palette[a].Blue = i;
      Palette[a].Reserved = 0;
      a++;
    }
  }
  else if(BitCount == 4)  // 16 ������ - ����������� ������� Windows
  {
    for(a = 0; a < 16; a++) {
    	Palette[a].Red = stdPalette16[a][0];
    	Palette[a].Green = stdPalette16[a][1];
    	Palette[a].Blue = stdPalette16[a][2];
		}
  }
  else if(BitCount == 1)    // ����������� �����������
  {
    Palette[0].Red = 0x00; Palette[0].Green = 0x00; Palette[0].Blue = 0x00;
    Palette[1].Red = 0xFF; Palette[1].Green = 0xFF; Palette[1].Blue = 0xFF;
  }

  return;
}



// --------------------------------------
class Image {
	_BITMAPINFOHEADER BMInfoHeader;
	_RGBTRIPLE *Rgbtriple;       // ����� �����������
	_RGBQUAD 	*Palette;         // �������

public:
	Image (BYTE Mode, WORD BCount, int Width, int Height);
  ~Image ();
	int loadImage(char *fileName);
	void writeImage(char *fileName); 							//  ������ ����������� � ����
  Image& copyImage(const Image &sourceImage);

  WORD getBitCount()
  {
    return BMInfoHeader.BitCount;
  }
  WORD getHeight()
  {
    return BMInfoHeader.Height;
  }
  WORD getWidth()
  {
    return BMInfoHeader.Width;
  }


	Image(char *fileName)						// ������������ ������� ����������� �� �����
	{
		this -> loadImage(fileName);
	}

	Image (const Image &sourceImage)				// ������������ �����
	{
		this->copyImage(sourceImage);
	}

	Image ()										// ������� ������ ��������� ��� �����������
	{
		BMInfoHeader.SizeImage = 0;
    BMInfoHeader.Size = 40;
    BMInfoHeader.Width = NULL;
    BMInfoHeader.Height = NULL;
    BMInfoHeader.Planes = NULL;
    BMInfoHeader.BitCount = NULL;
    BMInfoHeader.Compression = NULL;
    BMInfoHeader.SizeImage = 0;
    BMInfoHeader.XPelsPerMeter = NULL;
    BMInfoHeader.YPelsPerMeter = NULL;
    BMInfoHeader.ColorImportant = NULL;
    BMInfoHeader.ColorUsed = NULL;
	}

	Image& operator=(const Image& sourceImage);

  Image& operator /= (const Image& sourceImage);  // ���������� �������� � �������� sourceImage

  Image operator / (const WORD Depth);            // ��������� ��������
};
// ---------------


Image& Image::operator=(const Image& sourceImage)
	{
		if(this->BMInfoHeader.SizeImage == 0)     // ���� ����� ����������� ������, ������ �������� ������
		{
			this -> copyImage(sourceImage);
			return *this;
		}
		    // ����� ���������, ����� �� ����������� ���������� ������
		else if(this->BMInfoHeader.BitCount == sourceImage.BMInfoHeader.BitCount &&
            this->BMInfoHeader.Height == sourceImage.BMInfoHeader.Height &&
            this->BMInfoHeader.Width == sourceImage.BMInfoHeader.Width &&
            this->BMInfoHeader.Size == sourceImage.BMInfoHeader.Size &&
            this->BMInfoHeader.SizeImage == sourceImage.BMInfoHeader.SizeImage &&
            this->BMInfoHeader.Compression == sourceImage.BMInfoHeader.Compression &&
            this->BMInfoHeader.Planes == sourceImage.BMInfoHeader.Planes
            //this->BMInfoHeader.ColorUsed == sourceImage.BMInfoHeader.ColorUsed &&
            //this->BMInfoHeader.ColorImportant == sourceImage.BMInfoHeader.ColorImportant
            )
    {
      delete[] Rgbtriple;     // ������� ������ � ������ �����������-��������

      if(this->BMInfoHeader.BitCount < 24)    // ���� ��� ����������� � ��������, �� �������� �
      {
        delete[] Palette;
        Palette = new _RGBQUAD[(WORD)pow(2, BMInfoHeader.BitCount)];
        for(WORD i = 0; i < (WORD)pow(2, BMInfoHeader.BitCount); i++)
          Palette[i] = sourceImage.Palette[i];
      }

          // ������ � ��������� ������ ����� ������� �� �����������-���������
      Rgbtriple = new _RGBTRIPLE[BMInfoHeader.Width * BMInfoHeader.Height];

      for(DWORD i = 0; i < BMInfoHeader.Height; i++)
        for(DWORD j = 0; j < BMInfoHeader.Width; j++)
          Rgbtriple[i*BMInfoHeader.Width + j] = sourceImage.Rgbtriple[i*BMInfoHeader.Width + j];
    }

    return *this;
	}

  // �������� ������ ����������� �����������
Image::Image(BYTE Mode, WORD BCount, int Width, int Height)
{
    DWORD offsetBitesAtEnd = 0;

    if(((DWORD)ceil(Width * BCount / 8.0) % 4) > 0)
      offsetBitesAtEnd = 4 - (DWORD)ceil(Width * BCount / 8.0) % 4;

    BMInfoHeader.Size = 40;
    BMInfoHeader.Width = Width;
    BMInfoHeader.Height = Height;
    BMInfoHeader.Planes = 1;
    BMInfoHeader.BitCount = BCount;
    BMInfoHeader.Compression = 0;
    BMInfoHeader.SizeImage = (Height * (ceil(Width * BCount / 8.0) + offsetBitesAtEnd));
    BMInfoHeader.XPelsPerMeter = Width;
    BMInfoHeader.YPelsPerMeter = Height;
    BMInfoHeader.ColorImportant = 0;
    BMInfoHeader.ColorUsed = 0;

    if(BMInfoHeader.BitCount < 24)
    {
      // ��� ���������� ����������� Mode ����� ������� ����� � �������
      if(BMInfoHeader.BitCount == 1)
      {
        Palette = new _RGBQUAD[2];
        fillPalette(Palette, BMInfoHeader.BitCount);  // ���������� �������
        BMInfoHeader.ColorUsed = 2;
      }

      else if(BMInfoHeader.BitCount == 4)
      {
          // ������� - �������� ������
        Palette = new _RGBQUAD[16];
        fillPalette(Palette, BMInfoHeader.BitCount);  // ���������� �������
        BMInfoHeader.ColorUsed = 16;
      }

      // �������� "����������" ������� �� 216 ������
      else if(BMInfoHeader.BitCount == 8)
      {
        Palette = new _RGBQUAD[256];
        fillPalette(Palette, BMInfoHeader.BitCount);  // ���������� �������
        BMInfoHeader.ColorUsed = 256;
      }

      // ---------------------------

      Rgbtriple = new _RGBTRIPLE[BMInfoHeader.Height * BMInfoHeader.Width];
			// ���������� ������ �����������
			for (DWORD i = 0; i < BMInfoHeader.Height; i++)
				for (DWORD j = 0; j < BMInfoHeader.Width; j++)
					{
					  Rgbtriple[i*Width + j] = Palette[Mode];   // �������� ���� �� �������
					}
    }

		else if (BMInfoHeader.BitCount == 24 || BMInfoHeader.BitCount == 32)
		{
			// ��������� ������ ��� ����������� ������� �������� Height*Width ���� _RGBTRIPLE
			Rgbtriple = new _RGBTRIPLE[BMInfoHeader.Height * BMInfoHeader.Width];

			// ���������� ������ �����������
			for (DWORD i = 0; i < BMInfoHeader.Height; i++)
        for (DWORD j = 0; j < BMInfoHeader.Width; j++)
				{
					Rgbtriple[i*BMInfoHeader.Width + j].Red = Mode;
					Rgbtriple[i*BMInfoHeader.Width + j].Green = Mode;
					Rgbtriple[i*BMInfoHeader.Width + j].Blue = Mode;
				}
		}
}

  // ����������
Image::~Image()
{
	if(BMInfoHeader.BitCount == 24)	delete[] Rgbtriple;
	if(BMInfoHeader.BitCount < 24)		// ���� � ����������� ���� �������
	{
		delete[] Palette;
		delete[] Rgbtriple;
	}
}
// ----------------------


int Image::loadImage(char *fileName)
{
	_BITMAPFILEHEADER BMFileHeader;
  ifstream fin;
  DWORD offsetBitesAtEnd = 0;	// ���������� ������, ������� ����� �������� � ����� ������ (��. ������)

	fin.open(fileName, ios::binary);
	fin.read((char*)&BMFileHeader, sizeof(_BITMAPFILEHEADER));				// �������� ���� �� ��������� �����

	if(BMFileHeader.Type != 0x4D42)							// ��� �� BMP
	{
		cout << "The file is not BMP!\n";
		fin.close();
		exit(0);
		return 0;
	}

	fin.seekg(sizeof(_BITMAPFILEHEADER), ios_base::beg);			// ������������� ��������� ������ ����� ����� ���������
	fin.read((char*)&BMInfoHeader, sizeof(_BITMAPINFOHEADER));				// �������� ���� �� ��������� �����������


  // ���� ����� ������ �� ������ 4 ������, �� ����� ������ ������� 1, 2 ��� 3 ������� �����
  // ���������� ��� ���������� ������� ������
  if(((DWORD)ceil(BMInfoHeader.Width * BMInfoHeader.BitCount / 8.0) % 4) > 0)
    offsetBitesAtEnd = 4 - (DWORD)ceil(BMInfoHeader.Width * BMInfoHeader.BitCount / 8.0) % 4;

	// ���� ������������ �������
	if(BMInfoHeader.BitCount < 24)
	{
		fin.seekg(sizeof(_BITMAPFILEHEADER) + sizeof(_BITMAPINFOHEADER), ios_base::beg);   // ������ ��������� ����� ����������
		DWORD p_size = pow(2, BMInfoHeader.BitCount);							// ������ �������
		Palette = new _RGBQUAD[p_size];
		for(DWORD i = 0; i < p_size; i++)
			fin.read((char*)&Palette[i], sizeof(_RGBQUAD));								// ��������� �������
	}

	// ������ �����������

  // ���� ��� ����������� � ��������
  if(BMInfoHeader.BitCount < 24)
  {
      // ���������� (�����-�����) �����������
    if(BMInfoHeader.BitCount == 1)
    {
      BYTE mask = 0x80;     // 1000 0000
      BYTE tmp;
        // ��� ����� ����� ������ ����������� � ����� (� ������)
      DWORD bitLineLength = (DWORD)ceil(BMInfoHeader.Width / 8.0);
      DWORD j;

      Rgbtriple = new _RGBTRIPLE[BMInfoHeader.Height * BMInfoHeader.Width];

        // ������ ���� ����������� �������� ���������� � 8 ������
      for (DWORD i = 0; i < BMInfoHeader.Height; i++)
      {
        j = 0;

        for(DWORD bl = 0; bl < bitLineLength; bl++)
        {
          tmp = fin.get();    // ������ ��������� ���� �����������

          for(WORD k = 0; k < 8 && j < BMInfoHeader.Width; k++)    // �������� �� ������� ����
          {
            Rgbtriple[i * BMInfoHeader.Width + j++] = Palette[(bool)(tmp & (mask >> k))];
          }
        }

        if(offsetBitesAtEnd)		// ���� ����� �������� ����, �������� ������
          fin.seekg(offsetBitesAtEnd, ios_base::cur);
      }
    }

      // 4-������ �����������. � ������� 16 ������. ���� ���� ����������� �������� ���� � 2-� ������
    else if(BMInfoHeader.BitCount == 4)
    {
      BYTE mask = 0xF0;   // 11110000
      BYTE tmp;
      DWORD j;
      DWORD bitLineLength = (DWORD)ceil(BMInfoHeader.Width / 2.0);
      Rgbtriple = new _RGBTRIPLE[BMInfoHeader.Height * BMInfoHeader.Width];

      for(DWORD i = 0; i < BMInfoHeader.Height; i++)		// ������ ����
      {
        j = 0;
        for(DWORD bl = 0; bl < bitLineLength; bl++)		// ����� �������
        {
          tmp = fin.get();

            // tmp & mask ��������� �������� ������ 4 ����, ����� �������� �� ������ �� 4 �
            //  �������� ����, ���������� ����� ����� � �������
          Rgbtriple[i*BMInfoHeader.Width + j++] = Palette[(BYTE)(tmp & mask) >> 4];
            // (mask >> 4) - �������� ����� ������ � �������� 00001111
            // ����� �������� ����� ����� � ������� ��������� �����
          if(j < BMInfoHeader.Width)
            Rgbtriple[i*BMInfoHeader.Width + j++] = Palette[(BYTE)(tmp & (mask >> 4))];
        }

        if(offsetBitesAtEnd)		// ���� ����� �������� ����, �������� ������
          fin.seekg(offsetBitesAtEnd, ios_base::cur);
      }
    }

      // 8-������ �����������. � ������� 256 ������. � ����� ����� ���� �� ����� �����.
    else if(BMInfoHeader.BitCount == 8)
    {
      Rgbtriple = new _RGBTRIPLE[BMInfoHeader.Height * BMInfoHeader.Width];

      for(DWORD i = 0; i < BMInfoHeader.Height; i++)
      {
        for(DWORD j = 0; j < BMInfoHeader.Width; j++)
        {
          // �������� ���� �� �����, ������� �������� ������� ����� � �������
          Rgbtriple[i*BMInfoHeader.Width + j] = Palette[(BYTE)fin.get()];
        }

        if(offsetBitesAtEnd)		// ���� ����� �������� ����, �������� ������
          fin.seekg(offsetBitesAtEnd, ios_base::cur);
      }
	  }
  }
    // ���� ��� �������
  else
  {
    fin.seekg(BMFileHeader.OffsetBits, ios_base::beg);		// ������ ��������� �� �����������

    if(BMInfoHeader.BitCount == 24)
    {
      Rgbtriple = new _RGBTRIPLE[BMInfoHeader.Height * BMInfoHeader.Width];

      for(DWORD i = 0; i < BMInfoHeader.Height; i++)		// ������ ����
      {
        for(DWORD j = 0; j < BMInfoHeader.Width; j++)		// ����� �������
        {
        	fin.read((char*)&Rgbtriple[i*BMInfoHeader.Width + j], sizeof(_RGBTRIPLE));
        }

        if(offsetBitesAtEnd)		// ���� ����� �������� ����, �������� ������
          fin.seekg(offsetBitesAtEnd, ios_base::cur);
      }
    }

    else if(BMInfoHeader.BitCount == 32)
    {
      Rgbtriple = new _RGBTRIPLE[BMInfoHeader.Height*BMInfoHeader.Width];
      _RGBQUAD tmpQuad;

      for(DWORD i = 0; i < BMInfoHeader.Height; i++)		// ������ ����
      {
        for(DWORD j = 0; j < BMInfoHeader.Width; j++)		// ����� �������
        {
        	fin.read((char*)&tmpQuad, sizeof(_RGBQUAD));
          Rgbtriple[i*BMInfoHeader.Width + j] = tmpQuad;
        }
      }
    }
  }

  cout << "Load " << BMInfoHeader.BitCount << "-bit BMP file\n";
	fin.close();
	return 1;
}

// ---------------------------------------------

void Image::writeImage(char *fileName)
{
	if (BMInfoHeader.Size == 0) return;			// ���� ����������� �� ���������, ������ �������

	_BITMAPFILEHEADER BMFileHeader;
  ofstream fout;
	DWORD offsetBitesAtEnd = 0;
	BYTE tmp;

	fout.open(fileName, ios::binary);
	BMFileHeader.Type = 0x4D42;
  BMFileHeader.Reserved1 = 0;
  BMFileHeader.Reserved2 = 0;
	BMFileHeader.OffsetBits = sizeof(_BITMAPFILEHEADER) + sizeof(_BITMAPINFOHEADER);

	if(((DWORD)ceil(BMInfoHeader.Width * BMInfoHeader.BitCount / 8.0) % 4) > 0)
    offsetBitesAtEnd = 4 - (DWORD)ceil(BMInfoHeader.Width * BMInfoHeader.BitCount / 8.0) % 4;

    // ���� ���� �������, ��������� ��� ��� ����������� ������� � ������
	if (BMInfoHeader.BitCount < 24)
	{
		BMFileHeader.OffsetBits += pow(2, BMInfoHeader.BitCount) * sizeof(_RGBQUAD);
		//BMFileHeader.Size = (BMInfoHeader.Height * (ceil(BMInfoHeader.Width * BMInfoHeader.BitCount / 8.0) + offsetBitesAtEnd)) + BMFileHeader.OffsetBits;
	}

	BMFileHeader.Size = (BMInfoHeader.Height * (ceil(BMInfoHeader.Width * BMInfoHeader.BitCount / 8.0) + offsetBitesAtEnd)) + BMFileHeader.OffsetBits;
  //BMFileHeader.Size += offsetBitesAtEnd * BMInfoHeader.Height;

	cout << "recording...\n";

	fout.write((char*)&BMFileHeader, sizeof(_BITMAPFILEHEADER));
	fout.write((char*)&BMInfoHeader, sizeof(_BITMAPINFOHEADER));

  // ׸���-����� �����������
  if (BMInfoHeader.BitCount == 1)
  {
    // ����� �������
    for(DWORD i = 0; i < 2; i++)
    	fout.write((char*)&Palette[i], sizeof(_RGBQUAD));

    for (DWORD i = 0; i < BMInfoHeader.Height; i++)
    {
      for (DWORD j = 0; j < BMInfoHeader.Width; )
      {
        tmp = Rgbtriple[i * BMInfoHeader.Width + j++].Blue == 0xFF ? 1 : 0;    // R=G=B (���� 00, ���� FF), ���� �������� �� ������
        for(WORD k = 1; k < 8; k++)    // �� 8-�� ����� (������) ����� ������������ ���� ����
        {
          tmp <<= 1;
          if(j < BMInfoHeader.Width)
            tmp += Rgbtriple[i * BMInfoHeader.Width + j++].Blue == 0xFF ? 1 : 0;
        }
        fout.put(tmp);
      }

      for(DWORD k = 0; k < offsetBitesAtEnd; k++)
				fout.put(0);
    }
  }

  else if(BMInfoHeader.BitCount == 4)
  {
    // ����� ������� (16 ������)
    for(WORD i = 0; i < 16; i++)
    	fout.write((char*)&Palette[i], sizeof(_RGBQUAD));

    BYTE c;   // ����� ����� � �������

    for (DWORD i = 0; i < BMInfoHeader.Height; i++)
    {
      for (DWORD j = 0; j < BMInfoHeader.Width; )
      {
        // ��� ������� ����� ����������� �������� �� ���� ������� � ���� ����������
        for(c = 0; c < 16; c++)
          if(Rgbtriple[i * BMInfoHeader.Width + j] == Palette[c]) break;

        tmp = c < 16 ? c : 15;    // ���� ���� ������, ����� ��� ����� � �������
                                  // ����� (���� �� ������ ����) - ��������� ���� (�����)

        tmp <<= 4;      // �������� �����
        j++;

        if(j < BMInfoHeader.Width)
        {
          for(c = 0; c < 16; c++)   // �� �� ����� ��� ��������� �����
            if(Rgbtriple[i * BMInfoHeader.Width + j] == Palette[c]) break;

          tmp += (c < 16 ? c : 15);
          j++;
        }

				fout.put(tmp);
      }

      for(DWORD k = 0; k < offsetBitesAtEnd; k++)
      	fout.put(0);
    }
  }

	else if (BMInfoHeader.BitCount == 8)
	{
		for(WORD i = 0; i < 256; i++)
			fout.write((char*)&Palette[i], sizeof(_RGBQUAD));

    WORD c;   // ����� ����� � �������

		for (DWORD i = 0; i < BMInfoHeader.Height; i++)
    {
      for (DWORD j = 0; j < BMInfoHeader.Width; j++)
      {
        for(c = 0; c < 256; c++)
          if(Rgbtriple[i * BMInfoHeader.Width + j] == Palette[c]) break;

				fout.put(c < 256 ? c : 255);
      }

      for(DWORD k = 0; k < offsetBitesAtEnd; k++)
      	fout.put(0);
    }
	}

	else if (BMInfoHeader.BitCount == 24)
	{
		for(DWORD i = 0; i < BMInfoHeader.Height; i++)
		{
			for(DWORD j = 0; j < BMInfoHeader.Width; j++)
				{
					fout.write((char*)&Rgbtriple[i * BMInfoHeader.Width + j], sizeof(_RGBTRIPLE));
				}

			for(DWORD k = 0; k < offsetBitesAtEnd; k++)
				fout.put(0);
		}
	}

	else if (BMInfoHeader.BitCount == 32)
	{
	  _RGBQUAD tmpQuad;
		for(DWORD i = 0; i < BMInfoHeader.Height; i++)
		{
			for(DWORD j = 0; j < BMInfoHeader.Width; j++)
				{
				  tmpQuad = Rgbtriple[i * BMInfoHeader.Width + j];
				  fout.write((char*)&tmpQuad, sizeof(_RGBQUAD));
				}

			for(DWORD k = 0; k < offsetBitesAtEnd; k++)
				fout.put(0);
		}
	}

  cout << "Created " << BMInfoHeader.BitCount << "-bit BMP file: " << fileName << endl;
	fout.close();
	return;
}


Image& Image::copyImage (const Image& sourceImage)
{
    if(BMInfoHeader.Size == 0) return *this;

    BMInfoHeader.BitCount = sourceImage.BMInfoHeader.BitCount;
    BMInfoHeader.ColorImportant = sourceImage.BMInfoHeader.ColorImportant;
    BMInfoHeader.ColorUsed = sourceImage.BMInfoHeader.ColorUsed;
    BMInfoHeader.Compression = sourceImage.BMInfoHeader.Compression;
    BMInfoHeader.Height = sourceImage.BMInfoHeader.Height;
    BMInfoHeader.Width = sourceImage.BMInfoHeader.Width;
    BMInfoHeader.Planes = sourceImage.BMInfoHeader.Planes;
    BMInfoHeader.Size = sourceImage.BMInfoHeader.Size;
    BMInfoHeader.SizeImage = sourceImage.BMInfoHeader.SizeImage;
    BMInfoHeader.XPelsPerMeter = sourceImage.BMInfoHeader.XPelsPerMeter;
    BMInfoHeader.YPelsPerMeter = sourceImage.BMInfoHeader.YPelsPerMeter;

    // �������� �������
    if(BMInfoHeader.BitCount < 24)
    {
      Palette = new _RGBQUAD[(WORD)pow(2, BMInfoHeader.BitCount)];
      for(WORD i = 0; i < (WORD)pow(2, BMInfoHeader.BitCount); i++)
        Palette[i] = sourceImage.Palette[i];
    }

      // �������� ������
    Rgbtriple = new _RGBTRIPLE[BMInfoHeader.Width * BMInfoHeader.Height];

    for(DWORD i = 0; i < BMInfoHeader.Height; i++)
      for(DWORD j = 0; j < BMInfoHeader.Width; j++)
        Rgbtriple[i*BMInfoHeader.Width + j] = sourceImage.Rgbtriple[i*BMInfoHeader.Width + j];

    return *this;
}

    // ���������������
Image& Image::operator /= (const Image& sourceImage)
{
  if(BMInfoHeader.Size == 0) return *this;    // ������������ ������ ������������ �����������
            // ���� ������� ����� �� ���������, �� ���� �������
  if(BMInfoHeader.BitCount != sourceImage.BMInfoHeader.BitCount) return *this;

  cout << "Start scaling... Size of source image: "
	        << sourceImage.BMInfoHeader.Width << " x " << sourceImage.BMInfoHeader.Height << endl;

  // ��� ��������������� � �����������-��������� ���������� ������ ��� ������
  DWORD H = BMInfoHeader.Height;    // ������� ������
  DWORD W = BMInfoHeader.Width;     // ������� ������
  DWORD h = sourceImage.BMInfoHeader.Height;        // ������ �����������-���������
  DWORD w = sourceImage.BMInfoHeader.Width;         // ������ �����������-���������
  WORD r, g, b, cnt;     // ����� ����� ������������ �������� �����


  // �������� ������� ��������� ����������� �������� �����������-���������
  if(BMInfoHeader.BitCount < 24)
    for(WORD i = 0; i < pow(2, BMInfoHeader.BitCount); i++)
    {
      Palette[i] = sourceImage.Palette[i];
    }

  // ������������ ����������� ������� ��������������
  for(DWORD i = 0; i < H; i++)    // ������ ����
  {
    for(DWORD j = 0; j < W; j++)  // ����� �������
    {
      // ��� ������ ����� ������ ����������� ����� ����� � ����
      r = g = b = 0;
      cnt = 0;
        // ��� ������ ����� ������ ����������� ���� �� ����� ���������, ������� ���������� � ����
        //  � ������� ������� ��������
      for(DWORD k = floor(i*h/(double)H); k < ceil((i+1)*h/(double)H); k++)
        for(DWORD l = floor(j*w/(double)W); l < ceil((j+1)*w/(double)W); l++)
        {
          r += sourceImage.Rgbtriple[k * w + l].Red;
          g += sourceImage.Rgbtriple[k * w + l].Green;
          b += sourceImage.Rgbtriple[k * w + l].Blue;
          cnt++;
        }

      r /= cnt;
      g /= cnt;
      b /= cnt;

        // ���� ����������� ����������, �� ����� ������� ���� ������ ����� �� ����������� � �������
        // ������� ����� ����� ��������������� ��� ������������ ����
      if(BMInfoHeader.BitCount < 24)
      {
        WORD num;
        num = getColorInPalette(r, g, b, Palette, pow(2, BMInfoHeader.BitCount));
        Rgbtriple[i * W + j].Red = Palette[num].Red;
        Rgbtriple[i * W + j].Green = Palette[num].Green;
        Rgbtriple[i * W + j].Blue = Palette[num].Blue;
      }
      else      // ����� ������ ���������� ���� ����
      {
        Rgbtriple[i * W + j].Red = r;
        Rgbtriple[i * W + j].Green = g;
        Rgbtriple[i * W + j].Blue = b;
      }
    }
  }

  cout << "Scaling done. New image sizes: "
	     << BMInfoHeader.Width << " x " << BMInfoHeader.Height << endl;

  return *this;
}
// ------------------

// ��������� �������
Image Image::operator / (const WORD Depth)
{
  if(BMInfoHeader.BitCount == Depth)
  {
    cout << "The image already has " << Depth << " depth!\n";
    return *this;
  }
  if(Depth != 1 && Depth != 4 && Depth != 8 && Depth != 24 && Depth != 32)
  {
    cout << "Depth must be equals 1, 4, 8, 24 or 32!\n";
    return *this;
  }

    // ������ ����� �����������-�������
  Image newImage(0, Depth, BMInfoHeader.Width, BMInfoHeader.Height);

  if(Depth >= 24)
  {
    for(DWORD i = 0; i < BMInfoHeader.Height; i++)
      for(DWORD j = 0; j < BMInfoHeader.Width; j++)
        newImage.Rgbtriple[i*BMInfoHeader.Width + j] = Rgbtriple[i*BMInfoHeader.Width + j];
  }

  else    // ��������� ����������� ������ ����� � ����� ��������
  {
    for(DWORD i = 0; i < BMInfoHeader.Height; i++)
      for(DWORD j = 0; j < BMInfoHeader.Width; j++)
        newImage.Rgbtriple[i*BMInfoHeader.Width + j] =
            getColorInPalette(Rgbtriple[i*BMInfoHeader.Width + j], newImage.Palette, Depth);
  }


  cout << "Depth has been changed from " << BMInfoHeader.BitCount << " to " << Depth << ".\n";
  return newImage;
}



int main(int argc, char* argv[])
{
	char inputFileName[200] = "", outputFileName[200] = "output.bmp";
	BYTE Mode = -1;
	WORD BitCount = 0;
	DWORD Height = 0, Width = 0;
	bool createNewImage = false;

	if(argc < 2)
  {
    cout << "There is no input params. Use key /? for more information\n";
    return 0;
  }

  if(argc == 2 && strcmp(argv[1], "/?") == 0)
  {
    cout << "\nAV /command [argument]\n\n";
    cout << "List of commands:\n";
    cout << "  /input\t Name of input file (example: img.bmp)\n";
    cout << "  /output\t Name of output file (example: out.bmp)\n";
    cout << "  /createnew\t This key means that will be created a new image with one color (argument is not required)\n";
    cout << "  /height\t Height of output image\n";
    cout << "  /width\t Width of output image\n";
    cout << "  /bitcount\t Depth (bitcount) of output image\n";
    cout << "  /mode\t\t Used when creating a new image. \n";
    cout << "  \t\t For 24 and 32 bit images R=G=B=Mode, for 1, 4 and 8 bit images Mode is a number in palette.\n";
    return 0;
  }

  else      // ���� ���������� 2 � ������, �� ������������
  {
    for(int i = 1; i < argc; i++)
    {
      if(strcmp(argv[i], "/input") == 0)      // ��� �������� �����
      {
        if(argc < i + 1) { cout << "Incorrect format. Use /? for more information.\n"; return 0; }
        strcpy(inputFileName, argv[++i]);
      }
      else if(strcmp(argv[i], "/output") == 0)    // ��� ��������� �����
      {
        if(argc < i + 1) { cout << "Incorrect format. Use /? for more information.\n"; return 0; }
        strcpy(outputFileName, argv[++i]);
      }
      else if(strcmp(argv[i], "/height") == 0)    // ������ ��������� �����������
      {
        if(argc < i + 1) { cout << "Incorrect format. Use /? for more information.\n"; return 0; }
        Height = atoi(argv[++i]);
      }
      else if(strcmp(argv[i], "/width") == 0)    // ������ ��������� �����������
      {
        if(argc < i + 1) { cout << "Incorrect format. Use /? for more information.\n"; return 0; }
        Width = atoi(argv[++i]);
      }
      else if(strcmp(argv[i], "/bitcount") == 0 || strcmp(argv[i], "/depth") == 0)    // ������� ����� ��������� �����������
      {
        if(argc < i + 1) { cout << "Incorrect format. Use /? for more information.\n"; return 0; }
        BitCount = atoi(argv[++i]);
        if(BitCount != 0 && BitCount != 1 && BitCount != 4 && BitCount != 8 && BitCount != 24 && BitCount != 32)
          {
            cout << "BitCount must be 1, 4, 8, 24 or 32\n";
            return 0;
          }
      }
      else if(strcmp(argv[i], "/createnew") == 0)    // ����� ������� ����� ����������� � ����� ������
      {
        createNewImage = true;
      }
      else if(strcmp(argv[i], "/mode") == 0)
      {
        if(argc < i + 1)
        {
          cout << "Incorrect format. Use /? for more information.\n";
          return 0;
        }
        ++i;
        Mode = (atoi(argv[i]) <= 255) ? atoi(argv[i]) : 0;
      }
      else
      {
        cout << "Incorrect format. Use /? for more information.\n";
        return 0;
      }
    } // for
  // ------------------
  }
  // --------

  // ��������� �������� � ������������ � ���������� �����������
  if(createNewImage)    // �������� �������� � ����
  {
    // �� ��������� ��������� ���������� ���������� �� ���������
    if(Mode == -1) Mode = 0;
    if(BitCount == 0) BitCount = 24;
    if(Height == 0) Height = 100;
    if(Width == 0) Width = 100;

    Image inputImage(Mode, BitCount, Height, Width);
    inputImage.writeImage(outputFileName);
  }


  else if(strlen(inputFileName) == 0)
  {
    cout << "You need specify the input file! Use key /? for more information.\n";
  }
  else
  {
    Image inputImage = Image(inputFileName);
    Image outputImage;

    // ��������� ��������
    if(Height > 0 || Width > 0)
    {
      if(BitCount == 0)
        BitCount = inputImage.getBitCount();
        // ���� ������ ������ ������ ��� ������, �� ������ �������� ����� ����������������
      if(Height == 0) Height = round(Width * inputImage.getHeight() / inputImage.getWidth());
      if(Width == 0) Width = round(Height * inputImage.getWidth() / inputImage.getHeight());

      outputImage = Image(Mode, BitCount, Width, Height);

      if(BitCount != 0 && BitCount != inputImage.getBitCount())
        {
          cout << "Changing depth and scaling...\n";
          Image tmpImg = (inputImage / BitCount);
          outputImage /= tmpImg;   // ��������� �������� � �������
        }
      else
        {
          cout << "Scaling...\n";
          outputImage /= inputImage;    // ��������� ��������
        }
    }
    else if(BitCount != 0 && BitCount != inputImage.getBitCount())
    {
      cout << "Changing depth...\n";
      outputImage = (inputImage / BitCount);    // ��������� �������
    }
    else
    {
      cout << "Just coping...\n";
      outputImage = inputImage;         // ������� �����������
    }

    outputImage.writeImage(outputFileName);
  }

	return 0;
}

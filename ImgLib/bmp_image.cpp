#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <cstdint>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

PACKED_STRUCT_BEGIN BitmapFileHeader {
    char b;
    char m;
    uint32_t total_size;
    int32_t reserved_area;
    uint32_t headers_size;
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    uint32_t header_size;
    int32_t width;
    int32_t height;
    uint16_t count_planes;
    uint16_t bits_per_pixel;
    uint32_t compression_type;
    uint32_t data_size;
    int32_t horizontal_dpi;
    int32_t vertical_dpi;
    int32_t count_colors;
    int32_t count_significant_colors;
}
PACKED_STRUCT_END

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
    return 4 * ((w * 3 + 3) / 4);
}

static const int PPM_MAX = 255;

bool SaveBMP(const Path& file, const Image& image) {
    const int w = image.GetWidth();
    const int h = image.GetHeight();

    ofstream out(file, ios::binary);
    BitmapFileHeader bitmap_file_header;
    bitmap_file_header.b = 'B';
    bitmap_file_header.m = 'M';
    bitmap_file_header.reserved_area = 0;
    bitmap_file_header.headers_size = 54u;

    int otst = GetBMPStride(w);

    BitmapInfoHeader bitmap_info_header;
    bitmap_info_header.header_size = 40u;
    bitmap_info_header.width = w;
    bitmap_info_header.height = h;
    bitmap_info_header.count_planes = 1u;
    bitmap_info_header.bits_per_pixel = 24u;
    bitmap_info_header.compression_type = 0u;
    bitmap_info_header.data_size = static_cast<uint32_t>(otst * h);
    bitmap_info_header.horizontal_dpi = 11811;
    bitmap_info_header.vertical_dpi = 11811;
    bitmap_info_header.count_colors = 0;
    bitmap_info_header.count_significant_colors = 0x1000000;

    bitmap_file_header.total_size = bitmap_file_header.headers_size + bitmap_info_header.data_size;

    out.write(reinterpret_cast<char*>(&bitmap_file_header), sizeof(BitmapFileHeader));
    out.write(reinterpret_cast<char*>(&bitmap_info_header), sizeof(BitmapInfoHeader));

    std::vector<char> buff(otst);

    for (int y = h - 1; y >= 0; --y) {
        const Color* line = image.GetLine(y);
        for (int x = 0; x < w; ++x) {
            buff[x * 3 + 0] = static_cast<char>(line[x].b);
            buff[x * 3 + 1] = static_cast<char>(line[x].g);
            buff[x * 3 + 2] = static_cast<char>(line[x].r);
        }
        out.write(buff.data(), otst);
    }

    return out.good();
}

Image LoadBMP(const Path& file) {
    ifstream ifs(file, ios::binary);
    BitmapFileHeader bitmap_file_header;
    ifs.read(reinterpret_cast<char*>(&bitmap_file_header), sizeof(BitmapFileHeader));
    if (bitmap_file_header.b != 'B' || bitmap_file_header.m != 'M') {
        return {};
    }
    BitmapInfoHeader bitmap_info_header;
    ifs.read(reinterpret_cast<char*>(&bitmap_info_header), sizeof(BitmapInfoHeader));

    Image result(bitmap_info_header.width, bitmap_info_header.height, Color::Black());
    int otst = GetBMPStride(bitmap_info_header.width);
    std::vector<char> buff(otst);

    for (int y = bitmap_info_header.height - 1; y >= 0; --y) {
        Color* line = result.GetLine(y);
        ifs.read(buff.data(), otst);

        for (int x = 0; x < bitmap_info_header.width; ++x) {
            line[x].b = static_cast<byte>(buff[x * 3 + 0]);
            line[x].g = static_cast<byte>(buff[x * 3 + 1]);
            line[x].r = static_cast<byte>(buff[x * 3 + 2]);
        }
    }

    return result;
}

}  // namespace img_lib
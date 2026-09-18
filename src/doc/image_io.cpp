// Document Library
// Aseprite  | Copyright (C) 2001-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doc/image_io.h"

#include "base/base.h"
#include "base/exception.h"
#include "base/serialization.h"
#include "doc/image.h"
#include "zlib.h"

#include <iostream>
#include <memory>

namespace doc
{

using namespace base::serialization;
using namespace base::serialization::little_endian;

// TODO Create a zlib wrapper for iostreams

void write_image(std::ostream& os, const Image* image)
{
  write32(os, image->id());
  write8(os, image->pixelFormat()); // Pixel format
  write16(os, image->width());      // Width
  write16(os, image->height());     // Height
  write32(os, image->maskColor());  // Mask color

  const int rowSize = image->getRowStrideSize();
#if 0
  {
    for (int c=0; c<image->height(); c++)
      os.write((char*)image->getPixelAddress(0, c), rowSize);
  }
#else
  {
    const std::ostream::pos_type total_output_pos = os.tellp();
    write32(os, 0); // Compressed size (we update this value later)

    z_stream zstream;
    zstream.zalloc = nullptr;
    zstream.zfree = nullptr;
    zstream.opaque = nullptr;
    int err = deflateInit(&zstream, Z_DEFAULT_COMPRESSION);
    if (err != Z_OK)
      throw base::Exception("ZLib error %d in deflateInit().", err);

    std::vector<uint8_t> compressed(4096);
    int total_output_bytes = 0;

    for (int y = 0; y < image->height(); y++)
    {
      zstream.next_in = static_cast<Bytef*>(image->getPixelAddress(0, y));
      zstream.avail_in = rowSize;
      const int flush = (y == image->height() - 1 ? Z_FINISH : Z_NO_FLUSH);

      do
      {
        zstream.next_out = static_cast<Bytef*>(&compressed[0]);
        zstream.avail_out = compressed.size();

        // Compress
        err = deflate(&zstream, flush);
        if (err != Z_OK && err != Z_STREAM_END && err != Z_BUF_ERROR)
          throw base::Exception("ZLib error %d in deflate().", err);

        const int output_bytes = compressed.size() - zstream.avail_out;
        if (output_bytes > 0)
        {
          if (os.write(reinterpret_cast<char*>(&compressed[0]), output_bytes)
                  .fail())
            throw base::Exception("Error writing compressed image pixels.\n");

          total_output_bytes += output_bytes;
        }
      } while (zstream.avail_out == 0);
    }

    err = deflateEnd(&zstream);
    if (err != Z_OK)
      throw base::Exception("ZLib error %d in deflateEnd().", err);

    const std::ostream::pos_type bak = os.tellp();
    os.seekp(total_output_pos);
    write32(os, total_output_bytes);
    os.seekp(bak);
  }
#endif
}

Image* read_image(std::istream& is, bool setId)
{
  const ObjectId id = read32(is);
  const int pixelFormat = read8(is);     // Pixel format
  const int width = read16(is);          // Width
  const int height = read16(is);         // Height
  const uint32_t maskColor = read32(is); // Mask color

  if ((pixelFormat != IMAGE_RGB && pixelFormat != IMAGE_GRAYSCALE &&
       pixelFormat != IMAGE_INDEXED && pixelFormat != IMAGE_BITMAP) ||
      (width < 1 || height < 1) || (width > 0xfffff || height > 0xfffff))
    return nullptr;

  std::unique_ptr<Image> image(
      Image::create(static_cast<PixelFormat>(pixelFormat), width, height));
  const int rowSize = image->getRowStrideSize();

#if 0
  {
    for (int c=0; c<image->height(); c++)
      is.read((char*)image->getPixelAddress(0, c), rowSize);
  }
#else
  {
    const int avail_bytes = read32(is);

    z_stream zstream;
    zstream.zalloc = nullptr;
    zstream.zfree = nullptr;
    zstream.opaque = nullptr;

    int err = inflateInit(&zstream);
    if (err != Z_OK)
      throw base::Exception("ZLib error %d in inflateInit().", err);

    const int uncompressed_size = image->height() * rowSize;
    int uncompressed_offset = 0;
    int remain = avail_bytes;

    std::vector<uint8_t> compressed(4096);
    uint8_t* address = image->getPixelAddress(0, 0);
    const uint8_t* address_end =
        image->getPixelAddress(0, 0) + uncompressed_size;

    while (remain > 0)
    {
      const int len = MIN(remain, (int)compressed.size());
      if (is.read(reinterpret_cast<char*>(&compressed[0]), len).fail())
      {
        ASSERT(false);
        throw base::Exception("Error reading stream to restore image");
      }

      const int bytes_read = static_cast<int>(is.gcount());
      if (bytes_read == 0)
      {
        ASSERT(remain == 0);
        break;
      }

      remain -= bytes_read;

      zstream.next_in = static_cast<Bytef*>(&compressed[0]);
      zstream.avail_in = static_cast<uInt>(bytes_read);

      do
      {
        zstream.next_out = static_cast<Bytef*>(address);
        zstream.avail_out = address_end - address;

        err = inflate(&zstream, Z_NO_FLUSH);
        if (err != Z_OK && err != Z_STREAM_END && err != Z_BUF_ERROR)
          throw base::Exception("ZLib error %d in inflate().", err);

        const int uncompressed_bytes =
            static_cast<int>((address_end - address) - zstream.avail_out);
        if (uncompressed_bytes > 0)
        {
          if (uncompressed_offset + uncompressed_bytes > uncompressed_size)
            throw base::Exception("Bad compressed image.");

          uncompressed_offset += uncompressed_bytes;
          address += uncompressed_bytes;
        }
      } while (zstream.avail_in != 0 && zstream.avail_out == 0);
    }

    err = inflateEnd(&zstream);
    if (err != Z_OK)
      throw base::Exception("ZLib error %d in inflateEnd().", err);
  }
#endif

  image->setMaskColor(maskColor);
  if (setId)
    image->setId(id);
  return image.release();
}

} // namespace doc

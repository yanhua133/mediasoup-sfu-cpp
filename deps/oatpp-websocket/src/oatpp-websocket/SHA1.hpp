// [intaken src] https://github.com/vog/sha1
/*
     sha1.hpp - header of
 
     ============
     SHA-1 in C++
     ============
 
     100% Public Domain.
 
     Original C Code
     -- Steve Reid <steve@edmweb.com>
     Small changes to fit into bglibs
     -- Bruce Guenter <bruce@untroubled.org>
     Translation to simpler C++ Code
     -- Volker Diels-Grabsch <v@njh.eu>
     Safety fixes
     -- Eugene Hopkinson <slowriot at voxelstorm dot com>
     Adapted for oat++
     -- Leonid Stryzhevskyi <lganzzzo@gmail.com>
 */

#ifndef oatpp_websocket_SHA1_HPP
#define oatpp_websocket_SHA1_HPP

#include "oatpp/core/data/stream/ChunkedBuffer.hpp"
#include "oatpp/core/Types.hpp"

#include <cstdint>
#include <iostream>
#include <string>

namespace oatpp { namespace websocket {

/**
 * SHA1 implementation needed for websocket handshake key.
 */
class SHA1 {
private:
  typedef oatpp::data::stream::ChunkedBuffer ChunkedBuffer;
private:
  static constexpr size_t BLOCK_INTS = 16;  /* number of 32bit integers per SHA1 block */
  static constexpr size_t BLOCK_BYTES = BLOCK_INTS * 4;
private:
  static void reset(uint32_t digest[], ChunkedBuffer& buffer, uint64_t &transforms);
  static uint32_t rol(const uint32_t value, const size_t bits);
  static uint32_t blk(const uint32_t block[BLOCK_INTS], const size_t i);
  static void R0(const uint32_t block[BLOCK_INTS], const uint32_t v, uint32_t &w, const uint32_t x, const uint32_t y, uint32_t &z, const size_t i);
  static void R1(uint32_t block[BLOCK_INTS], const uint32_t v, uint32_t &w, const uint32_t x, const uint32_t y, uint32_t &z, const size_t i);
  static void R2(uint32_t block[BLOCK_INTS], const uint32_t v, uint32_t &w, const uint32_t x, const uint32_t y, uint32_t &z, const size_t i);
  static void R3(uint32_t block[BLOCK_INTS], const uint32_t v, uint32_t &w, const uint32_t x, const uint32_t y, uint32_t &z, const size_t i);
  static void R4(uint32_t block[BLOCK_INTS], const uint32_t v, uint32_t &w, const uint32_t x, const uint32_t y, uint32_t &z, const size_t i);
  static void transform(uint32_t digest[], uint32_t block[BLOCK_INTS], uint64_t &transforms);
  static void buffer_to_block(ChunkedBuffer& buffer, uint32_t block[BLOCK_INTS]);
private:
  uint32_t digest[5];
  oatpp::data::stream::ChunkedBuffer buffer;
  uint64_t transforms;
public:

  /**
   * Constructor.
   */
  SHA1();

  /**
   * Update digest.
   * @param s - &id:oatpp::String;.
   */
  void update(const oatpp::String& s);

  /**
   * Update digest from `std::istream`.
   * @param is - `std::istream`.
   */
  void update(std::istream& is);

  /**
   * Add padding and return the message digest.
   * @return - &id:oatpp::String;.
   */
  oatpp::String finalBinary();

  /**
   * Get digest of the file content.
   * @param filename - name of the file. &id:oatpp::String;.
   * @return - &id:oatpp::String;.
   */
  static oatpp::String fromFile(const oatpp::String& filename);
};

}}
  
#endif /* oatpp_websocket_SHA1_HPP */

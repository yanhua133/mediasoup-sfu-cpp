/***************************************************************************
 *
 * Project:   ______                ______ _
 *           / _____)              / _____) |          _
 *          | /      ____ ____ ___| /     | | _   ____| |_
 *          | |     / _  |  _ (___) |     | || \ / _  |  _)
 *          | \____( ( | | | | |  | \_____| | | ( ( | | |__
 *           \______)_||_|_| |_|   \______)_| |_|\_||_|\___)
 *
 *
 * Copyright 2020-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#ifndef Nickname_hpp
#define Nickname_hpp

#include "oatpp/core/Types.hpp"

#include <vector>
#include <random>

class Nickname {
public:

  static constexpr int AVATARS_SIZE = 70;
  static const char* const AVATARS[];

  static constexpr int ADJECTIVES_SIZE = 103;
  static const char* const ADJECTIVES[];

  static constexpr int NOUNS_SIZE = 49;
  static const char* NOUNS[];

private:
  static thread_local std::mt19937 RANDOM_GENERATOR;
public:

  static oatpp::String random();

};

#endif // Nickname_hpp

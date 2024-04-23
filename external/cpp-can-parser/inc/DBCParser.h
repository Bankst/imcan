#pragma once

#include "CANObjects.h"
#include "Tokenizer.h"
#include <set>
#include <memory>

namespace CppCAN {
namespace parsing {
namespace dbc {

CANDatabase fromTokenizer(
  const std::string& name, details::Tokenizer& tokenizer,
  std::vector<CANDatabase::parsing_warning>* warnings = nullptr);

CANDatabase fromTokenizer(
  details::Tokenizer& tokenizer, 
  std::vector<CANDatabase::parsing_warning>* warnings = nullptr);
  
} // namespace CppCAN
} // namespace parsing
} // namespace dbc

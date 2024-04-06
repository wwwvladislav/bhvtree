/*
 Copyright (c) 2024 Vladislav Volkov <wwwvladislav@gmail.com>

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include "bhvserializer.hpp"
#include "bhvtree.hpp"
#include <stdexcept>
#include <string>
#include <string_view>

namespace cppttl {
namespace bhv {
namespace {
using namespace std::string_literals;

namespace lex {
static const auto none = "none"s;
static const auto pred = "pred"s;
static const auto then_ = "then"s;
static const auto else_ = "else"s;
static const auto case_ = "case"s;
static const auto default_ = "default"s;
static const auto body = "body"s;
} // namespace lex

const char *type_name(node const &n) { return to_string(n.type()); }

std::string mask(std::string const &str) {
  auto it = str.find('"');
  if (it == std::string::npos)
    return str;

  std::string res;
  res.reserve(str.size() + 8);

  std::string::size_type p = 0;

  for (; it != std::string::npos; it = str.find('"', it)) {
    res += str.substr(p, it - p);
    res += '\\';
    p = it;
    it += 1;
  }

  res += str.substr(p);

  return res;
}

std::string node_name(node const &n) {
  return "\""s + mask(std::string(n.name())) + "\""s;
}

class serializer final {
public:
  serializer(std::ostream &stream);

  serializer &operator<<(node const &ref);

private:
  std::ostream &indent(size_t layer) const;
  void save(basic_control::childs_list const &list, size_t layer) const;
  void save(node const &ref, size_t layer, std::string_view prefix = "") const;
  void save(action const &ref, size_t layer,
            std::string_view prefix = "") const;
  void save(condition const &ref, size_t layer,
            std::string_view prefix = "") const;
  void save(sequence const &ref, size_t layer,
            std::string_view prefix = "") const;
  void save(fallback const &ref, size_t layer,
            std::string_view prefix = "") const;
  void save(parallel const &ref, size_t layer,
            std::string_view prefix = "") const;
  void save(if_ const &ref, size_t layer, std::string_view prefix = "") const;
  void save(switch_ const &ref, size_t layer,
            std::string_view prefix = "") const;
  void save(invert const &ref, size_t layer,
            std::string_view prefix = "") const;
  void save(repeat const &ref, size_t layer,
            std::string_view prefix = "") const;
  void save(retry const &ref, size_t layer, std::string_view prefix = "") const;
  void save(force const &ref, size_t layer, std::string_view prefix = "") const;

private:
  std::ostream &_stream;
};

serializer::serializer(std::ostream &stream) : _stream(stream) {}

serializer &serializer::operator<<(node const &ref) {
  save(ref, 0);
  return *this;
}

std::ostream &serializer::indent(size_t layer) const {
  static const char *spaces = "  ";
  for (size_t i = 0; i < layer; ++i) {
    _stream << spaces;
  }
  return _stream;
}

void serializer::save(basic_control::childs_list const &list,
                      size_t layer) const {
  for (auto &child : list)
    save(*child, layer, "- ");
}

void serializer::save(node const &ref, size_t layer,
                      std::string_view prefix) const {
  switch (ref.type()) {
  case node_type::action:
    save(static_cast<action const &>(ref), layer, prefix);
    break;
  case node_type::condition:
    save(static_cast<condition const &>(ref), layer, prefix);
    break;
  case node_type::sequence:
    save(static_cast<sequence const &>(ref), layer, prefix);
    break;
  case node_type::fallback:
    save(static_cast<fallback const &>(ref), layer, prefix);
    break;
  case node_type::parallel:
    save(static_cast<parallel const &>(ref), layer, prefix);
    break;
  case node_type::if_:
    save(static_cast<if_ const &>(ref), layer, prefix);
    break;
  case node_type::switch_:
    save(static_cast<switch_ const &>(ref), layer, prefix);
    break;
  case node_type::invert:
    save(static_cast<invert const &>(ref), layer, prefix);
    break;
  case node_type::repeat:
    save(static_cast<repeat const &>(ref), layer, prefix);
    break;
  case node_type::retry:
    save(static_cast<retry const &>(ref), layer, prefix);
    break;
  case node_type::force:
    save(static_cast<force const &>(ref), layer, prefix);
    break;
  case node_type::custom:
    throw std::runtime_error("Unsupported node type");
    break;
  }
}

void serializer::save(action const &ref, size_t layer,
                      std::string_view prefix) const {
  indent(layer) << prefix << type_name(ref) << " " << node_name(ref)
                << std::endl;
}

void serializer::save(condition const &ref, size_t layer,
                      std::string_view prefix) const {
  indent(layer) << prefix << type_name(ref) << " " << node_name(ref)
                << std::endl;
}

void serializer::save(sequence const &ref, size_t layer,
                      std::string_view prefix) const {
  indent(layer) << prefix << type_name(ref) << " " << node_name(ref) << ":";

  if (!ref.childs().empty()) {
    _stream << std::endl;
    save(ref.childs(), layer + 1);
  } else {
    _stream << " " << lex::none << std::endl;
  }
}

void serializer::save(fallback const &ref, size_t layer,
                      std::string_view prefix) const {
  indent(layer) << prefix << type_name(ref) << " " << node_name(ref) << ":";

  if (!ref.childs().empty()) {
    _stream << std::endl;
    save(ref.childs(), layer + 1);
  } else {
    _stream << " " << lex::none << std::endl;
  }
}

void serializer::save(parallel const &ref, size_t layer,
                      std::string_view prefix) const {
  indent(layer) << prefix << type_name(ref) << " threshold=" << ref.threshold()
                << " " << node_name(ref) << ":";

  if (!ref.childs().empty()) {
    _stream << std::endl;
    save(ref.childs(), layer + 1);
  } else {
    _stream << " " << lex::none << std::endl;
  }
}

void serializer::save(if_ const &ref, size_t layer,
                      std::string_view prefix) const {
  indent(layer) << prefix << type_name(ref) << " " << node_name(ref) << ":";

  if (!ref.childs().empty()) {
    _stream << std::endl;

    if (ref.condition()) {
      save(*ref.condition(), layer + 1, lex::pred + ": ");
    }
    if (ref.then_()) {
      save(*ref.then_(), layer + 1, lex::then_ + ": ");
    }
    if (ref.else_()) {
      save(*ref.else_(), layer + 1, lex::else_ + ": ");
    }
  } else {
    _stream << " " << lex::none << std::endl;
  }
}

void serializer::save(switch_ const &ref, size_t layer,
                      std::string_view prefix) const {
  indent(layer) << prefix << type_name(ref) << " " << node_name(ref) << ":";

  if (!ref.empty()) {
    _stream << std::endl;

    node::cptr handler;

    for (auto &&case_ : ref) {
      if (handler && handler != case_.handler())
        save(*handler, layer + 2, lex::body + ": ");
      save(*case_.condition(), layer + 1, "- " + lex::case_ + ": ");
      handler = case_.handler();
    }

    if (handler)
      save(*handler, layer + 2, lex::body + ": ");

    if (ref.default_handler()) {
      save(*ref.default_handler(), layer + 1, "- " + lex::default_ + ": ");
    }
  } else {
    _stream << " " << lex::none << std::endl;
  }
}

void serializer::save(invert const &ref, size_t layer,
                      std::string_view prefix) const {
  indent(layer) << prefix << type_name(ref) << " " << node_name(ref) << ": ";

  if (!ref.childs().empty()) {
    save(*ref.childs().front(), 0);
  } else {
    _stream << lex::none << std::endl;
  }
}

void serializer::save(repeat const &ref, size_t layer,
                      std::string_view prefix) const {
  indent(layer) << prefix << type_name(ref) << " n=" << ref.count() << " "
                << node_name(ref) << ": ";

  if (!ref.childs().empty()) {
    save(*ref.childs().front(), 0);
  } else {
    _stream << lex::none << std::endl;
  }
}

void serializer::save(retry const &ref, size_t layer,
                      std::string_view prefix) const {
  indent(layer) << prefix << type_name(ref) << " n=" << ref.count() << " "
                << node_name(ref) << ": ";

  if (!ref.childs().empty()) {
    save(*ref.childs().front(), 0);
  } else {
    _stream << lex::none << std::endl;
  }
}

void serializer::save(force const &ref, size_t layer,
                      std::string_view prefix) const {
  indent(layer) << prefix << type_name(ref)
                << " status=" << to_string(ref.result()) << " "
                << node_name(ref) << ": ";

  if (!ref.childs().empty()) {
    save(*ref.childs().front(), 0);
  } else {
    _stream << lex::none << std::endl;
  }
}

} // namespace

std::ostream &operator<<(std::ostream &stream, node const &ref) {
  serializer ss(stream);
  ss << ref;
  return stream;
}

} // namespace bhv
} // namespace cppttl

#include <emscripten/bind.h>
#include <string>
#include <utility>
#include <vector>
#include "oniguruma.h"

namespace onigjs {
  namespace {
    template<typename T, typename A>
    std::pair<UChar const *, UChar const *> uchar_range(std::vector<T, A> const &v) {
      return {reinterpret_cast<UChar const *>(v.data()),
          reinterpret_cast<UChar const *>(v.data() + v.size())};
    }

    std::vector<char16_t> repack_utf16(std::wstring const &s) {
      return std::vector<char16_t>(s.begin(), s.end());
    }
  }

  struct MatchResult {
    bool matched;
    size_t offset;
    size_t size;
  };

  struct RegExpOptions {
    bool global;
    bool ignoreCase;
    bool multiline;
    bool extended;

    RegExpOptions():
      global(false), ignoreCase(false), multiline(false), extended(false) {}

    RegExpOptions(std::string const &flags): RegExpOptions() {
      for(auto c: flags) {
        switch(c) {
        case 'g':
          global = true;
          break;
        case 'i':
          ignoreCase = true;
          break;
        case 'm':
          multiline = true;
          break;
        case 'x':
          extended = true;
          break;
        default:
          // ignore unknown flags
          break;
        }
      }
    }

    OnigOptionType asCompileOptions() const {
      OnigOptionType opt = ONIG_OPTION_NONE;
      if(ignoreCase) ONIG_OPTION_ON(opt, ONIG_OPTION_IGNORECASE);
      if(multiline) ONIG_OPTION_ON(opt, ONIG_OPTION_MULTILINE);
      if(extended) ONIG_OPTION_ON(opt, ONIG_OPTION_EXTEND);
      return opt;
    };

  };

  class RegExp {
    std::wstring const pattern;
    OnigRegex regex;
    OnigRegion region;
    RegExpOptions const options;

  public:
    RegExp(std::wstring const &pattern, std::string const &flags):
      pattern(pattern), regex(nullptr), region(), options(flags) {
      auto pattern16 = repack_utf16(pattern);
      auto pattern_range = uchar_range(pattern16);

      OnigErrorInfo errinfo;
      auto err = onig_new(&regex, pattern_range.first, pattern_range.second,
                          options.asCompileOptions(),
                          &OnigEncodingUTF16_LE,
                          &OnigSyntaxRuby, &errinfo);

      if(err != ONIG_NORMAL) {
        UChar errbuf[ONIG_MAX_ERROR_MESSAGE_LEN];
        auto len = onig_error_code_to_str(errbuf, err, &errinfo);
        throw std::string(reinterpret_cast<char const *>(errbuf), len);
      }

      onig_region_init(&region);
    }

    ~RegExp() {
      onig_free(regex);
      onig_region_free(&region, false);
    }

    MatchResult exec(std::wstring const &target) {
      auto target16 = repack_utf16(target);
      auto target_range = uchar_range(target16);

      auto position = onig_search(regex, target_range.first, target_range.second,
                                  target_range.first, target_range.second,
                                  &region, ONIG_OPTION_NONE);

      if(position == ONIG_MISMATCH) {
        return {false, 0, 0};
      } else {
        auto length = region.end[0] - region.beg[0];
        return {true, position / sizeof(char16_t), length / sizeof(char16_t)};
      }
    }

    bool global() const {
      return options.global;
    }

    bool ignoreCase() const {
      return options.ignoreCase;
    }

    bool multiline() const {
      return options.multiline;
    }

    std::wstring source() const {
      return pattern;
    }

    static std::string version() {
      return onig_version();
    }

    static std::string copyright() {
      return onig_copyright();
    }
  };
}

using namespace emscripten;
EMSCRIPTEN_BINDINGS(onigjs) {
  value_object<onigjs::MatchResult>("MatchResult")
    .field("matched", &onigjs::MatchResult::matched)
    .field("offset", &onigjs::MatchResult::offset)
    .field("size", &onigjs::MatchResult::size)
    ;

  class_<onigjs::RegExp>("RegExp")
    .constructor<std::wstring, std::string>()
    .function("exec", &onigjs::RegExp::exec)
    .property("global", &onigjs::RegExp::global)
    .property("ignoreCase", &onigjs::RegExp::ignoreCase)
    .property("multiline", &onigjs::RegExp::multiline)
    .property("source", &onigjs::RegExp::source)
    .class_function("version", &onigjs::RegExp::version)
    .class_function("copyright", &onigjs::RegExp::copyright)
    ;
}

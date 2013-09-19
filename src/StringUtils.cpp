/*
 Copyright (C) 2010-2013 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "StringUtils.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <locale>

namespace StringUtils {
    struct CaseSensitiveCharCompare {
    public:
        int operator()(const char& lhs, const char& rhs) const {
            return lhs - rhs;
        }
    };
    
    struct CaseInsensitiveCharCompare {
    private:
        std::locale m_locale;
    public:
        CaseInsensitiveCharCompare(std::locale loc = std::locale::classic()) :
        m_locale(loc) {}
        
        int operator()(const char& lhs, const char& rhs) const {
            return std::tolower(lhs, m_locale) - std::tolower(rhs, m_locale);
        }
    };
    
    template <typename Cmp>
    struct CharEqual {
    private:
        Cmp m_compare;
    public:
        bool operator()(const char& lhs, const char& rhs) const {
            return m_compare(lhs, rhs) == 0;
        }
    };
    
    template <typename Cmp>
    struct CharLess {
    private:
        Cmp m_compare;
    public:
        bool operator()(const char& lhs, const char& rhs) const {
            return m_compare(lhs, rhs) < 0;
        }
    };
    
    template <typename Cmp>
    struct StringEqual {
    public:
        bool operator()(const String& lhs, const String& rhs) const {
            if (lhs.size() != rhs.size())
                return false;
            return std::equal(lhs.begin(), lhs.end(), rhs.begin(), CharEqual<Cmp>());
        }
    };
    
    template <typename Cmp>
    struct StringLess {
        bool operator()(const String& lhs, const String& rhs) const {
            typedef String::iterator::difference_type StringDiff;
            
            String::const_iterator lhsEnd, rhsEnd;
            const size_t minSize = std::min(lhs.size(), rhs.size());
            StringDiff difference = static_cast<StringDiff>(minSize);
            
            std::advance(lhsEnd = lhs.begin(), difference);
            std::advance(rhsEnd = rhs.begin(), difference);
            return std::lexicographical_compare(lhs.begin(), lhsEnd, rhs.begin(), rhsEnd, CharLess<Cmp>());
        }
    };
    String formatString(const char* format, va_list arguments) {
        static char buffer[4096];
        
#if defined _MSC_VER
        vsprintf_s(buffer, format, arguments);
#else
        vsprintf(buffer, format, arguments);
#endif
        
        return buffer;
    }
    
    String trim(const String& str, const String& chars) {
        if (str.length() == 0)
            return str;
        
        size_t first = str.find_first_not_of(chars.c_str());
        if (first == String::npos)
            return "";
        
        size_t last = str.find_last_not_of(chars.c_str());
        if (first > last)
            return "";
        
        return str.substr(first, last - first + 1);
    }
    
    bool isPrefix(const String& str, const String& prefix) {
        if (prefix.empty())
            return true;
        if (prefix.size() > str.size())
            return false;
        
        for (size_t i = 0; i < prefix.size(); i++)
            if (prefix[i] != str[i])
                return false;
        return true;
    }
    
    bool containsCaseSensitive(const String& haystack, const String& needle) {
        return std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(), CharEqual<CaseSensitiveCharCompare>()) != haystack.end();
    }
    
    bool containsCaseInsensitive(const String& haystack, const String& needle) {
        return std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(),  CharEqual<CaseInsensitiveCharCompare>()) != haystack.end();
    }
    
    void sortCaseSensitive(StringList& strs) {
        std::sort(strs.begin(), strs.end(), StringLess<CaseSensitiveCharCompare>());
    }
    
    void sortCaseInsensitive(StringList& strs) {
        std::sort(strs.begin(), strs.end(), StringLess<CaseInsensitiveCharCompare>());
    }
    
    bool caseSensitiveEqual(const String& str1, const String& str2) {
        StringEqual<CaseSensitiveCharCompare> equality;
        return equality(str1, str2);
    }
    
    bool caseInsensitiveEqual(const String& str1, const String& str2) {
        StringEqual<CaseInsensitiveCharCompare> equality;
        return equality(str1, str2);
    }
    
    long makeHash(const String& str) {
        long hash = 0;
        String::const_iterator it, end;
        for (it = str.begin(), end = str.end(); it != end; ++it)
            hash = static_cast<long>(*it) + (hash << 6) + (hash << 16) - hash;
        return hash;
    }
    
    String toLower(const String& str) {
        String result(str);
        std::transform(result.begin(), result.end(), result.begin(), tolower);
        return result;
    }
    
    String replaceChars(const String& str, const String& needles, const String& replacements) {
        if (needles.size() != replacements.size() || needles.empty() || str.empty())
            return str;
        
        String result = str;
        for (size_t i = 0; i < needles.size(); ++i) {
            if (result[i] == needles[i])
                result[i] = replacements[i];
        }
        return result;
    }
    
    String capitalize(String str) {
        StringStream buffer;
        bool initial = true;
        for (unsigned int i = 0; i < str.size(); i++) {
            char c = str[i];
            if (c == ' ' || c == '\n' || c == '\t' || c == '\r') {
                initial = true;
                buffer << c;
            } else if (initial) {
                char d = static_cast<char>(toupper(c));
                buffer << d;
                initial = false;
            } else {
                buffer << c;
                initial = false;
            }
        }
        return buffer.str();
    }
    
}

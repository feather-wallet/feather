#include "brute.h"

brute::brute(const std::string &chars) {
    this->chars = chars;
    this->len = chars.length();
}

std::string brute::next() {
    std::string out;
    int c = count;
    int i;
    while (c >= 0){
        i = (c % len);
        out = chars[i] + out;
        c = (c / len) - 1;
    }
    count += 1;
    return out;
}

bruteword::bruteword(const std::string &chars) {
    this->m_chars = chars;
    this->m_currentStrategy = strategy::ReplaceSingle;
}

void bruteword::setWord(const std::string &word) {
    this->m_currentStrategy = strategy::ReplaceSingle;
    m_word = word;
    resetIndex();
}

void bruteword::nextStrategy() {
    m_currentStrategy = static_cast<strategy>(static_cast<int>(m_currentStrategy) + 1);
    resetIndex();
}

void bruteword::resetIndex() {
    iword = 0;
    ichar = 0;
}

std::string bruteword::next() {
    std::string out = m_word;

    switch(m_currentStrategy) {
        case strategy::ReplaceSingle: {
            if (iword >= m_word.length()) {
                this->nextStrategy();
                return next();
            }
            out[iword] = m_chars[ichar];
            ichar++;
            if (ichar == m_chars.length()){
                ichar = 0;
                iword++;
            }
            return out;
        }

        case strategy::AppendSingle: {
            if (ichar >= m_chars.length()) {
                this->nextStrategy();
                return next();
            }
            out += m_chars[ichar];
            ichar++;
            return out;
        }

        case strategy::PrependSingle: {
            if (ichar >= m_chars.length()) {
                this->nextStrategy();
                return next();
            }
            out = m_chars[ichar] + out;
            ichar++;
            return out;
        }

        case strategy::SwitchLetters: {
            if (iword+1 >= out.length()) {
                this->nextStrategy();
                return next();
            }
            char l = out[iword];
            out[iword] = out[iword+1];
            out[iword+1] = l;
            iword++;
            return out;
        }

        case strategy::DeleteSingle: {
            if (iword >= out.length()) {
                this->nextStrategy();
                return next();
            }
            out = out.substr(0, iword) + out.substr(iword+1);
            iword++;
            return out;
        }

        case strategy::AddSingle: {
            if (iword+1 >= out.length()) {
                this->nextStrategy();
                return next();
            }
            out = out.substr(0, iword+1) + m_chars[ichar] + out.substr(iword+1);
            ichar++;
            if (ichar == m_chars.length()) {
                ichar = 0;
                iword++;
            }
            return out;
        }

        case strategy::END: {
            return "";
        }
    }

    return "";
}
#pragma once

#include "Choice.g.h"

namespace winrt::OpenAI::implementation
{
  struct Choice : ChoiceT<Choice>
  {
    Choice() = default;
    winrt::hstring Text() const { return m_text; }
    winrt::hstring m_text;
  };
}


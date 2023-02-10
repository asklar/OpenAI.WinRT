#pragma once

#include "Choice.g.h"

namespace winrt::OpenAI::implementation
{

  struct Choice : ChoiceT<Choice>
  {
    Choice() = default;
    winrt::hstring Text() const noexcept { return m_text; }
    winrt::hstring m_text;

    OpenAI::FinishReason FinishReason() const noexcept { return m_finishReason; }
    OpenAI::FinishReason m_finishReason;
  };
}


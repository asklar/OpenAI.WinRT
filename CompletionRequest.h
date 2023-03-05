#pragma once

#include "CompletionRequest.g.h"
#include "ChatRequest.g.h"
#include <numeric>

namespace winrt::OpenAI::implementation
{
    struct CompletionRequest : CompletionRequestT<CompletionRequest>
    {
        CompletionRequest() = default;
        Property<winrt::hstring> Prompt;
        Property<winrt::hstring> Model;
        Property<winrt::hstring> Suffix;
        Property<uint32_t> MaxTokens = 16;
        Property<double> Temperature = 1;
        Property<double> TopP = 1;
        Property<uint8_t> NCompletions = 1;
        Property<bool> Stream;
        Property<uint8_t> LogProbs;
        Property<bool> Echo = false;
        Property<winrt::hstring> Stop;
        Property<double> PresencePenalty;
        Property<double> FrequencyPenalty;
        Property<uint8_t> BestOf = 1;
    };

    struct ChatRequest : ChatRequestT<ChatRequest>
    {
      ChatRequest() = default;
      Property<winrt::hstring> Model{ L"gpt-3.5-turbo" };
      Property<winrt::Windows::Foundation::Collections::IVector<winrt::OpenAI::ChatMessage>> Messages{ winrt::single_threaded_vector<winrt::OpenAI::ChatMessage>() };
      Property<uint32_t> MaxTokens{ std::numeric_limits<uint32_t>::infinity() };
      Property<double> Temperature = 1;
      Property<double> TopP = 1;
      Property<uint8_t> NCompletions = 1;
      Property<bool> Stream = false;

    };
}

namespace winrt::OpenAI::factory_implementation
{
    struct CompletionRequest : CompletionRequestT<CompletionRequest, implementation::CompletionRequest> {};
    struct ChatRequest : ChatRequestT<ChatRequest, implementation::ChatRequest> {};
}

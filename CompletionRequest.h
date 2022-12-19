#pragma once

#include "CompletionRequest.g.h"

namespace winrt::OpenAI::implementation
{
    struct CompletionRequest : CompletionRequestT<CompletionRequest>
    {
        CompletionRequest() = default;

        hstring Prompt() { return m_prompt; }
        void Prompt(hstring const& value) { m_prompt = value; }
        hstring Model() { return m_model; }
        void Model(hstring const& value) { m_model = value; }
        hstring Suffix() { return m_suffix; }
        void Suffix(hstring const& value) { m_suffix = value; }
        uint32_t MaxTokens() { return m_maxTokens; }
        void MaxTokens(uint32_t value) { m_maxTokens = value; }
        double Temperature() { return m_temperature; }
        void Temperature(double value) { m_temperature = value; }
        double TopP() { return m_topP; }
        void TopP(double value) { m_topP = value; }
        uint8_t NCompletions() { return m_nCompletions; }
        void NCompletions(uint8_t value) { m_nCompletions = value; }
        bool Stream() { return m_stream; }
        void Stream(bool value) { m_stream = value; }
        uint8_t LogProbs() { return m_logProbs; }
        void LogProbs(uint8_t value) { m_logProbs = value; }
        bool Echo() { return m_echo; }
        void Echo(bool value) { m_echo = value; }
        hstring Stop() { return m_stop; }
        void Stop(hstring const& value) { m_stop = value; }
        double PresencePenalty() { return m_presencePenalty; }
        void PresencePenalty(double value) { m_presencePenalty = value; }
        double FrequencyPenalty() { return m_frequencyPenalty; }
        void FrequencyPenalty(double value) { m_frequencyPenalty = value; }
        uint8_t BestOf() { return m_bestOf; }
        void BestOf(uint8_t value) { m_bestOf = value; }
    private:
      winrt::hstring m_model;
      winrt::hstring m_prompt;
      winrt::hstring m_suffix;
      uint32_t m_maxTokens{ 16 };
      double m_temperature{ 1 };
      double m_topP{ 1 };
      uint8_t m_nCompletions{ 1 };
      bool m_stream{};
      uint8_t m_logProbs{};
      bool m_echo{ false };
      winrt::hstring m_stop;
      double m_presencePenalty{};
      double m_frequencyPenalty{};
      uint8_t m_bestOf{1};


    };
}

namespace winrt::OpenAI::factory_implementation
{
    struct CompletionRequest : CompletionRequestT<CompletionRequest, implementation::CompletionRequest>
    {
    };
}

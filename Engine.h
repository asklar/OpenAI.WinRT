#pragma once

#include "Engine.g.h"
#include "Answer.g.h"

#include <future>

namespace winrt::OpenAI::implementation
{
  struct Answer : AnswerT<Answer>
  {
    Answer() = default;

    winrt::hstring Value() const noexcept { return m_value; }
    void Value(winrt::hstring const& value) { m_value = value; }
    double Confidence() const noexcept { return m_confidence; }
    void Confidence(double value) { m_confidence = value; }

  private:
    winrt::hstring m_value;
    double m_confidence;
  };

  struct Engine : EngineT<Engine>
  {
    Engine() = default;

    winrt::OpenAI::SearchEndpoint SearchEndpoint() const noexcept { return m_search; }
    void SearchEndpoint(winrt::OpenAI::SearchEndpoint const& value) { m_search = value; }
    winrt::OpenAI::OpenAIClient OpenAIClient() const noexcept { return m_client; }
    void OpenAIClient(winrt::OpenAI::OpenAIClient const& value) { m_client = value; }
    winrt::Windows::Foundation::IAsyncOperation<winrt::OpenAI::Answer> AskAsync(winrt::hstring query);

    winrt::event_token EngineStepSend(winrt::Windows::Foundation::TypedEventHandler<winrt::OpenAI::Engine, winrt::OpenAI::EngineStepEventArgs> const& handler) { return m_send.add(handler); }
    void EngineStepSend(winrt::event_token const& token) noexcept { m_send.remove(token); }
    winrt::event_token EngineStepReceive(winrt::Windows::Foundation::TypedEventHandler<winrt::OpenAI::Engine, winrt::OpenAI::EngineStepEventArgs> const& handler) { return m_receive.add(handler); }
    void EngineStepReceive(winrt::event_token const& token) noexcept {m_receive.remove(token); }

    uint32_t MaxSteps() const noexcept { return m_maxSteps; }
    void MaxSteps(uint32_t v) { m_maxSteps = v; }
    
  private:
    winrt::OpenAI::SearchEndpoint m_search;
    winrt::OpenAI::OpenAIClient m_client;

    winrt::Windows::Foundation::IAsyncOperation<winrt::OpenAI::Answer> AnswerFromSearchFlowAsync(winrt::hstring question);
    winrt::Windows::Foundation::IAsyncOperation<winrt::OpenAI::Answer> CalculatorAsync(winrt::hstring question);
    winrt::Windows::Foundation::IAsyncOperation<winrt::hstring> GetTitlesFromSearchResultsAsync(winrt::hstring question);
    winrt::event<winrt::Windows::Foundation::TypedEventHandler<winrt::OpenAI::Engine, winrt::OpenAI::EngineStepEventArgs>> m_send;
    winrt::event<winrt::Windows::Foundation::TypedEventHandler<winrt::OpenAI::Engine, winrt::OpenAI::EngineStepEventArgs>> m_receive;
    uint32_t m_maxSteps{ 5 };


  };
}

namespace winrt::OpenAI::factory_implementation
{
    struct Engine : EngineT<Engine, implementation::Engine>
    {
    };
}

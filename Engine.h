#pragma once

#include "Engine.g.h"
#include "Answer.g.h"
#include "Skill.g.h"

namespace winrt::OpenAI::implementation
{
  struct Answer : AnswerT<Answer>
  {
    Answer() = default;
    Answer(winrt::hstring value) { Value(value); }

    winrt::hstring Value() const noexcept { return m_value; }
    void Value(winrt::hstring const& value) { m_value = value; }
    double Confidence() const noexcept { return m_confidence; }
    void Confidence(double value) { m_confidence = value; }

  private:
    winrt::hstring m_value;
    double m_confidence;
  };

  struct Skill : SkillT<Skill>
  {
    Skill(winrt::hstring name, SkillHandlerAsync handler) { m_name = name; m_handler = handler; }

    winrt::hstring Name() const noexcept { return m_name; }
    winrt::Windows::Foundation::IAsyncOperation<winrt::OpenAI::Answer> ExecuteAsync(hstring query, hstring originalQuestion) { return m_handler(query, originalQuestion, m_engine); }
    OpenAI::Engine Engine() const noexcept { return m_engine; }
    void Engine(const OpenAI::Engine& e) { m_engine = e; }
    winrt::hstring m_name;
    SkillHandlerAsync m_handler;
    winrt::OpenAI::Engine m_engine{ nullptr };
  };

  struct Engine : EngineT<Engine>
  {
    Engine();

    winrt::Windows::Foundation::Collections::IVector<winrt::OpenAI::ISkill> Skills() { return m_skills; }

    winrt::Windows::Foundation::IAsyncOperation<winrt::OpenAI::Answer> AskAsync(winrt::hstring query);

    winrt::event_token EngineStepSend(winrt::Windows::Foundation::TypedEventHandler<winrt::OpenAI::Engine, winrt::OpenAI::EngineStepEventArgs> const& handler) { return m_send.add(handler); }
    void EngineStepSend(winrt::event_token const& token) noexcept { m_send.remove(token); }
    winrt::event_token EngineStepReceive(winrt::Windows::Foundation::TypedEventHandler<winrt::OpenAI::Engine, winrt::OpenAI::EngineStepEventArgs> const& handler) { return m_receive.add(handler); }
    void EngineStepReceive(winrt::event_token const& token) noexcept {m_receive.remove(token); }

    uint32_t MaxSteps() const noexcept { return m_maxSteps; }
    void MaxSteps(uint32_t v) { m_maxSteps = v; }

    winrt::OpenAI::ISkill GetSkill(winrt::hstring name) {
      auto s = std::find_if(m_skills.begin(), m_skills.end(), [name](const ISkill& s) {return s.Name() == name; });
      if (s != m_skills.end()) return *s;
      throw winrt::hresult_class_not_available{};
    }

    void ConnectSkills() const {
      for (const auto& s : m_skills) s.Engine(*this);
    }

  private:
    ISkill Search() {return GetSkill(L"search"); }

    ISkill Client() { return GetSkill(L"openai"); }

    winrt::Windows::Foundation::Collections::IVector<winrt::OpenAI::ISkill> m_skills{ nullptr };
    //winrt::Windows::Foundation::IAsyncOperation<winrt::OpenAI::Answer> AnswerFromSearchFlowAsync(winrt::hstring question, winrt::hstring original);
    //winrt::Windows::Foundation::IAsyncOperation<winrt::OpenAI::Answer> CalculatorAsync(winrt::hstring question, winrt::hstring original);
    //winrt::Windows::Foundation::IAsyncOperation<winrt::hstring> GetTitlesFromSearchResultsAsync(winrt::hstring question, winrt::hstring original);
    winrt::event<winrt::Windows::Foundation::TypedEventHandler<winrt::OpenAI::Engine, winrt::OpenAI::EngineStepEventArgs>> m_send;
    winrt::event<winrt::Windows::Foundation::TypedEventHandler<winrt::OpenAI::Engine, winrt::OpenAI::EngineStepEventArgs>> m_receive;
    uint32_t m_maxSteps{ 5 };


  };
}


namespace winrt::OpenAI::factory_implementation
{
    struct Engine : EngineT<Engine, implementation::Engine>{};

    struct Answer : AnswerT<Answer, implementation::Answer>{};

    struct Skill : SkillT<Skill, implementation::Skill> {};

}

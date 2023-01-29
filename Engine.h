#pragma once

#include "Engine.g.h"
#include "Answer.g.h"
#include "Skill.g.h"
#include "Context.g.h"
#include "EngineStepEventArgs.g.h"

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
    winrt::Windows::Foundation::IAsyncOperation<winrt::OpenAI::Answer> ExecuteAsync(hstring query, OpenAI::Context context) { return m_handler(query, context, m_engine); }
    OpenAI::Engine Engine() const noexcept { return m_engine; }
    void Engine(const OpenAI::Engine& e) { m_engine = e; }
    winrt::hstring m_name;
    SkillHandlerAsync m_handler;
    winrt::OpenAI::Engine m_engine{ nullptr };
  };

  struct Context : ContextT<Context>
  {
    Context() {
      m_basket = winrt::single_threaded_map<winrt::guid, IInspectable>(std::unordered_map<winrt::guid, IInspectable>{});
    }
    winrt::hstring OriginalQuestion() const noexcept { return m_originalQuestion; }
    uint32_t Step() const noexcept { return m_step; }
    winrt::Windows::Foundation::Collections::IMap<winrt::guid, IInspectable> Basket() { return m_basket; }
    

    winrt::hstring m_originalQuestion;
    uint32_t m_step{};
    winrt::Windows::Foundation::Collections::IMap<winrt::guid, IInspectable> m_basket{ nullptr };
      
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
    winrt::event_token EventLogged(winrt::Windows::Foundation::TypedEventHandler<winrt::hstring, winrt::hstring> const& handler) { return m_eventLogged.add(handler); }
    void EventLogged(winrt::event_token const& token) noexcept { m_eventLogged.remove(token); }


    uint32_t MaxSteps() const noexcept { return m_maxSteps; }
    void MaxSteps(uint32_t v) { m_maxSteps = v; }

    void Log(LogLevel level, winrt::hstring skill, winrt::hstring message) {
      if (m_eventLogged)
        m_eventLogged(skill, message);
      //std::wcout << L"[" << skillName << L" - " << (int32_t)level << L"] " << message << L"\n";
    }

    winrt::OpenAI::ISkill GetSkill(winrt::hstring name) {
      auto s = std::find_if(m_skills.begin(), m_skills.end(), [name](const ISkill& s) {
        return CompareStringOrdinal(s.Name().c_str(), -1, name.c_str(), -1, TRUE) == CSTR_EQUAL; 
        });

      if (s != m_skills.end()) {
        auto result = *s;
        return result;
      }
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
    winrt::event<winrt::Windows::Foundation::TypedEventHandler<winrt::hstring, winrt::hstring>> m_eventLogged;
    uint32_t m_maxSteps{ 5 };
  };

    struct EngineStepEventArgs : EngineStepEventArgsT<EngineStepEventArgs>
    {
      EngineStepEventArgs() = default;

      winrt::OpenAI::Context Context() { return m_context; }
      
      winrt::hstring EndpointName() { return m_endpointName; }
      
      winrt::hstring Value() { return m_value; }
      
      OpenAI::Context m_context{ nullptr };
      winrt::hstring m_endpointName;
      winrt::hstring m_value;
    };

}


namespace winrt::OpenAI::factory_implementation
{
    struct Engine : EngineT<Engine, implementation::Engine>{};

    struct Answer : AnswerT<Answer, implementation::Answer>{};

    struct Skill : SkillT<Skill, implementation::Skill> {};

}

#include "pch.h"
#include "Engine.h"
#if __has_include("Engine.g.cpp")
#include "Engine.g.cpp"
#endif
#include "Answer.g.cpp"
#include "Skill.g.cpp"
#include "Context.g.cpp"

#include <winrt/Windows.Data.Json.h>
#include <set>
#include <numeric>

#include <Calculator.h>

using namespace winrt::Windows::Data::Json;

namespace winrt::OpenAI::implementation
{


  Engine::Engine() {
    m_skills = winrt::single_threaded_vector<ISkill>({});
  }

  auto StringFromJson(JsonObject o, hstring key)
  {
    std::wstring value;
    auto v = o.Lookup(key);

    switch (v.ValueType()) {
    case JsonValueType::String:
      value = v.GetString(); break;
    case JsonValueType::Array:
    {
      auto arr = v.GetArray();
      value = std::accumulate(arr.begin(), arr.end(), std::wstring{ L"[" }, [](const auto& i, const auto& v) { return i + L",\"" + v.GetString() + L"\""; }).erase(1, 1) + L"]";
      break;
    }
    default:
      value = v.Stringify();
      break;

    }
    return value;
  }

  auto CreateStepArgs(OpenAI::Context context, std::wstring_view toolName, std::wstring_view question)
  {
    auto stepArgs = winrt::make<EngineStepEventArgs>();
    auto sa = winrt::get_self<EngineStepEventArgs>(stepArgs);
    sa->m_context = context;
    sa->m_endpointName = winrt::hstring{ toolName };
    sa->m_value = winrt::hstring{ question };
    return stepArgs;
  }

  winrt::hstring GetMimeType(winrt::Windows::Foundation::IInspectable o) {
    if (o.try_as<winrt::hstring>()) {
      return L"text/plain";
    }
    auto cn = winrt::get_class_name(o);
    if (cn == L"Windows.UI.Xaml.Media.Imaging.Bitmap" || cn == L"Windows.Graphics.Imaging.SoftwareBitmap") {
      return L"image/png";
    }
    return L"application/octet-stream";
  }

  winrt::Windows::Foundation::IAsyncOperation<winrt::OpenAI::Answer> Engine::ProcessJsonResponseAsync(JsonObject completionJson, winrt::OpenAI::implementation::Context* context) {
    winrt::OpenAI::Context context_{ *context };
    if (completionJson.HasKey(L"answer")) {
      auto a = winrt::make<Answer>();
      auto answer = StringFromJson(completionJson, L"answer");
      a.Value(answer);
      a.Confidence(completionJson.GetNamedNumber(L"confidence"));
      if (m_receive) { m_receive(*this, CreateStepArgs(context_, L"OpenAI", a.Value())); }
      co_return a;
    }
    else if (completionJson.HasKey(L"tool")) {
      auto tool = completionJson.GetNamedString(L"tool");
      winrt::hstring value{ StringFromJson(completionJson, L"input") };

      //} = completionJson.GetNamedString(L"value");
      if (m_send) { m_send(*this, CreateStepArgs(context_, tool, value)); }
      winrt::OpenAI::Answer bestResult{ nullptr };

      if (tool == L"ShowBasket") {
        std::wstring result;
        for (const auto& [k, v] : context_.Basket()) {
          if (result != L"") result += L", ";
          result += std::vformat(LR"({{ "id": "{}", "type": "{}" }})",
            std::make_wformat_args(winrt::to_hstring(k), GetMimeType(v)));
        }
        result = LR"({ "basket": [)" + result + L"]}\n";
        bestResult = OpenAI::Answer(result);
      } else {
        // try {
        auto skill = GetSkill(tool);
        auto v{ value };
        bestResult = co_await skill.ExecuteAsync(v, context_);
      }
      context->history += completionJson.Stringify() + L"\n";
      context->history += L"TOOL: " + bestResult.Value() + L"\n";
      //} catch (...){}

      if (m_receive) { m_receive(*this, CreateStepArgs(context_, tool, bestResult.Value())); }

    }
    context->m_step++;

  }
  winrt::Windows::Foundation::IAsyncOperation<winrt::OpenAI::Answer> Engine::AskAsync(winrt::hstring question, winrt::Windows::Foundation::Collections::IMap<winrt::guid, IInspectable> basket)
  {
    auto context_ = winrt::make<Context>();
    auto context = winrt::get_self<Context>(context_);
    context->m_basket = basket;
    
    std::set<std::wstring> questions;

    auto skillTemplate = LR"({{ "tool": "{}" }})";
    std::wstring skillsJson;
    for (const auto& skill : m_skills) {
      if (skill.Name() == L"openai") continue;
      auto snippet = std::vformat(skillTemplate, std::make_wformat_args(skill.Name()));
      if (skillsJson != L"") skillsJson += L", ";
      skillsJson += snippet;
    }

    constexpr std::wstring_view mainQueryTemplate{ 
LR"(You are part of a conversation with the user and a set of tools which you can call to help you answer user questions. 
The system that orchestrates the conversation system has a memory ("basket") that can store items of different types (files, images, documents, text). 
When the user refers to "this file" or "that image", they are referring to items in this basket.

Your job is to break down the problem into individual steps. To answer the user question, we will chain the results from previous steps. 
You will reply with either:
- one call to one tool (only the first tool) in json format in the format: {{ "tool": the tool name , "input": the input to the tool }}. 
- one answer, in json format in the format:   {{ "answer": "...", "confidence": ... }}. The confidence is a number between 0 and 1 about how confident you are in your answer.
- If you cannot figure out what tool to call, reply with: {{ "error": "explanation of the error" }}

When you call a tool, the tool will give you an answer and you will be asked the original question again, this time with the additional answer from the tool. Stop after the first json.
The tools will reply with "TOOL:" followed by a json like: {{ "tool": "toolName", "output": "..." }}.
If you give a tool the wrong kind of input, it will reply with "TOOL:" {{ "error": "an explanation of what happened" }}
When you are called again, you should adjust your call to the tool to fix the problem.

The user is interacting with a window that has content (text and images). 
The content can be obtained by taking a screenshot of the window with the GetWindow tool which will store the screenshot in the basket and return its id.

Here are the available tools with examples of possible input to each and what they each return:
{{ "tools": [
  {}
  {{ "name": "ShowBasket", "inputType": "{{}}", "description": "returns a json of the contents of the basket, which is a json array that has id and type, user context is stored in the basket  and you can use this tool to access it." }},
  {{ "name": "GetWindow", "inputType": "{{}}", "description": "takes a screenshot of the window and returns its id in the basket" }}
  ] }}

When picking an item from the basket, make sure you pick one with the right type per the user's question.

Do not repeat the user question.
The user question: {}

{}
<|endoftext|>
)" };

    // other skills that could be added:
    /*
      {{ "name": "AnalyzeImage", "inputType": "an id returned by ShowMemory", "description": "returns a json with the description of the image whose ID was passed in as input" }},

    */

    while (context->m_step < m_maxSteps || m_maxSteps == -1) {
      auto cr = winrt::OpenAI::CompletionRequest{};
      auto mainQuery = std::vformat(mainQueryTemplate, std::make_wformat_args(skillsJson, question, context->history));
      
      auto stepArgs = CreateStepArgs(context_, L"OpenAI", question);
      if (m_send && context->m_step == 0) m_send(*this, stepArgs);
      auto completion = co_await Client().ExecuteAsync(mainQuery, context_);
      
      auto completionStr = completion.Value();
      JsonObject completionJson;
      if (!JsonObject::TryParse(completionStr, completionJson)) {
        std::wstring firstLine{ completionStr };
        auto newline = firstLine.find('\n');
        if (newline != std::wstring::npos) {
          firstLine = firstLine.substr(0, newline);
        }
        completionStr = firstLine;
        JsonObject::TryParse(completionStr, completionJson);
      }

      if (completionJson.Size() != 0) {
        auto answer = co_await ProcessJsonResponseAsync(completionJson, context);
        if (answer != nullptr) {
          co_return answer;
        }
      } else {
        auto lastResponse = completion.Value();
        Log(OpenAI::LogLevel::Error, L"Engine", L"invalid response from GPT: " + lastResponse);
        throw winrt::hresult_invalid_argument{};
      }
    }


  }

  winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Data::Json::JsonObject> Engine::ParseApiInputAsync(hstring input, winrt::Windows::Foundation::Collections::IVector<winrt::OpenAI::Parameter> params)
  {
    std::wstring paramsStr = LR"({ "params": {)";
    constexpr auto Q = L"\"";
    auto i = 0;
    for (const auto& p : params) {
      if (i != 0) paramsStr += L", ";
      paramsStr += Q + p.Name + Q + L": " + Q + p.Type.Name + Q;
      i++;
    }
    paramsStr += L"}}";
    auto templateQuery = LR"(You are an AI assisting an API to parse its inputs from a call. 
The API provides its description in the form of a json with an inputs object that contains pairs of parameter name and expected parameter type.
For example, given the input string "123, 456, 789" and the API description {{ "inputs": {{ "bloop": "Int32", "flap": "Double", "brak": "String" }} }},
you would return the json {{ "bloop": 123, "flap": 456.0, "brak": "789" }}

The API description:
{}

The call: {}

Reply with the inputs in the expected json format and nothing else.
)";
    auto query = std::vformat(templateQuery, std::make_wformat_args(paramsStr, input));

    auto completion = co_await Client().ExecuteAsync(query, nullptr);
    auto json = JsonObject::Parse(completion.Value());
    co_return json;
  }
}

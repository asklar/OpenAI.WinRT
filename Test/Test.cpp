﻿// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/openai.h>
#include <winrt/builders/OpenAI.h>
#include <winrt/builders/helpers.h>
#include <winrt/Windows.Data.Json.h>
#include <numeric>
#include <format>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include "../Calculator.h"
#include <Windows.h>
using namespace winrt;
using namespace Windows::Data::Json;

winrt::Windows::Foundation::IAsyncOperation<winrt::OpenAI::Answer> CalculatorAsync(winrt::hstring expression, OpenAI::Context context, OpenAI::Engine engine) {
  auto a = OpenAI::Answer{};

  try {
    auto x = calculator::eval<double>(winrt::to_string(expression));
    a.Value(winrt::to_hstring(x));
    a.Confidence(1);
    co_return a;
  }
  catch (...) {}

  auto prompt = std::vformat(LR"(You are an AI designed to calculate arithmetic expressions. Reply with the simplest answer in the form of a json: {{ "answer": "...", "confidence": ... }}.
The confidence is a number between 0 and 1 about how confident you are in your answer.
The expression is: {}
)", std::make_wformat_args(expression.c_str()));


  auto client = engine.GetSkill(L"openai");
  auto completion = co_await client.ExecuteAsync(prompt, context);
  auto completion1 = completion.Value();
  JsonObject completionJson;
  if (JsonObject::TryParse(completion1, completionJson)) {
    if (completionJson.HasKey(L"answer")) {
      a.Value(completionJson.GetNamedString(L"answer"));
      a.Confidence(completionJson.GetNamedNumber(L"confidence"));
    }
  }
  co_return a;
}

using namespace winrt;
namespace wfc = winrt::Windows::Foundation::Collections;
namespace wf = winrt::Windows::Foundation;
template<typename T> using async = wf::IAsyncOperation<T>;

async<OpenAI::Answer> Sort(const hstring& expression, const OpenAI::Context& context, const OpenAI::Engine& engine) {

  auto client = engine.GetSkill(L"openai");
  auto input = co_await engine.ParseApiInputAsync(expression, single_threaded_vector<OpenAI::Parameter>(
    {
    OpenAI::Parameter{L"items", xaml_typename<wfc::IVector<hstring>>()}
    }));

  auto eng = engine;
  eng.Log(OpenAI::LogLevel::Informational, winrt::hstring{ L"sortAlphabetical" }, input.Stringify());
  JsonObject obj;
  std::vector<std::wstring> items;
  auto arr = input.GetNamedArray(L"items");
  for (const auto& a : arr) items.push_back(a.GetString().c_str());

  std::sort(items.begin(), items.end());
  std::wstring out = std::accumulate(items.begin(), items.end(), std::wstring{ L"[" }, 
    [](const auto& i, const auto& v) { return i + L",\"" + v  + L"\""; }).erase(1, 1) + L"]";
  auto answer = OpenAI::Answer(winrt::hstring{ out.c_str() });
  co_return answer;
}

WINRT_EXPORT namespace winrt
{
  template <typename T>
  inline Windows::UI::Xaml::Interop::TypeName xaml_typename2()
  {
    //static_assert(impl::has_category_v<T>, "T must be WinRT type.");
    static const Windows::UI::Xaml::Interop::TypeName name{ hstring{ impl::xaml_typename_name<T>::value() }, impl::xaml_typename_kind<T>::value };
    return name;
  }
}

int main()
{
  winrt::init_apartment(/*winrt::apartment_type::multi_threaded*/);



  auto intType = winrt::xaml_typename2<int32_t>();
  auto vectorOfInspectableType = winrt::xaml_typename2<winrt::Windows::Foundation::Collections::IVector<winrt::Windows::Foundation::IInspectable>>();

  auto x = 0;


  auto searchEndpoint = winrt::OpenAI::builders::SearchEndpoint();

  auto openaiEndpoint = winrt::OpenAI::builders::OpenAIClient()
    .CompletionUri(winrt::Windows::Foundation::Uri{ L"https://lrsopenai.openai.azure.com/openai/deployments/Text-Davinci3-Deployment/completions?api-version=2022-12-01" })
    .UseBearerTokenAuthorization(false)
    ;

  auto calculator = winrt::OpenAI::Skill(L"calculator", OpenAI::SkillHandlerAsync{ &CalculatorAsync });

  auto sort = winrt::OpenAI::Skill(L"sortListAlphabetical", { &Sort });

  auto files = winrt::OpenAI::Skill(L"files", [](winrt::hstring expression, OpenAI::Context context, OpenAI::Engine engine) -> winrt::Windows::Foundation::IAsyncOperation<winrt::OpenAI::Answer> {
    auto client = engine.GetSkill(L"openai");
    std::wstring exp{ expression };
    auto intent = co_await client.ExecuteAsync(std::vformat(LR"(You are an assistant helping the user with their files on Windows. 

Respond with a well-formed json that contains the folder and the set of files to fetch. Use * for wildcards.
For example: {{ "folder": "documents", "filespec": "*" }} to fetch all files from the documents folder.
or {{ "folder": "pictures", "filespec": "*.png" }} to fetch all png files.

Here are the files the user wants: {}

)", std::make_wformat_args(exp)), context);
    auto text = intent.Value();
    JsonObject json;
    if (JsonObject::TryParse(text, json)) {
      std::wcout << "[files] " << text << "\n";
      co_return OpenAI::Answer(LR"(hammerthrow.txt
taylorSwiftTopHits.docx
AgneHammerThrowRecord.md
OpenAIMonetizationPlan.pptx
ASklarIndieMovie.mp4
)");
    } else {
      co_return OpenAI::Answer(LR"({ "error": "input was not a valid json")");
    }
  });


  
  auto SetForegroundColor = [](uint32_t r) {
    return std::vformat(L"\033[38;2;{};{};{}m", std::make_wformat_args(GetRValue(r), GetGValue(r), GetBValue(r)));
  }; 
  
  std::wcout << SetForegroundColor(RGB(0xf3, 0x4f, 0x1c)) << L"\xdb";// \x2588";
  std::wcout << SetForegroundColor(RGB(0x7F, 0xBC, 0x00)) << L"\xdb";
  std::wcout << L"\033[0m"; 
  std::wcout << " Microsoft AI\n";

  std::wcout << SetForegroundColor(RGB(0xFF,0xBA, 0x01)) << L"\xdb";
  std::wcout << SetForegroundColor(RGB(0x01, 0xA6, 0xF0)) << L"\xdb";
  std::wcout << L"\033[0m"; 
  std::wcout << L" " << std::wstring(12, (wchar_t)196) << L"\n\n";

  std::wcout << SetForegroundColor(RGB(0x28,0x68,0xff)) << L"\033[1m";
  std::wcout << "What can I do for you? ";
  std::wcout << L"\033[0m";
  
  std::wstring question;
  std::getline(std::wcin, question);


  auto engine = winrt::OpenAI::builders::Engine{}
    .Skills({ 
      OpenAI::SearchSkill(searchEndpoint),
      OpenAI::GPTSkill(openaiEndpoint),
      calculator, 
      files, 
      sort,
  });
  engine.ConnectSkills();

  // For debugging purposes:
  engine.EngineStepSend([](const auto& engine, const winrt::OpenAI::EngineStepEventArgs& args) {
        std::wcout << L"Step " << args.Context().Step() << L" --> [" << args.EndpointName() << L"] " << args.Value() << L"\n";
      });
  engine.EngineStepReceive([](const auto& engine, const winrt::OpenAI::EngineStepEventArgs& args) {
    std::wcout << L"Step " << args.Context().Step() << L" <-- [" << args.EndpointName() << L"] " << args.Value() << L"\n";
    });
  engine.EventLogged([](winrt::hstring skill, winrt::hstring msg) {
    std::wcout << L"[" << skill << "]: " << msg << L"\n";
    });
  

  auto question =
    L"get the files from my desktop folder and sort them alphabetically"; // list the text items in the basket"
    ;
  //auto sf = winrt::Windows::Storage::KnownFolders::PicturesLibrary().GetFileAsync(L"IMG_20210726_164128.jpg").get();
  auto sf = winrt::Windows::Storage::KnownFolders::PicturesLibrary().GetFileAsync(L"BlueStacks_ScreenShot.jpg").get();
  auto stream = sf.OpenReadAsync().get();
  auto bmpDecoder = winrt::Windows::Graphics::Imaging::BitmapDecoder::CreateAsync(stream).get();
  auto bmp = bmpDecoder.GetSoftwareBitmapAsync().get();
    //L"get the files on my desktop folder and sort them alphabetically";
  
  auto answer = engine.AskAsync({ question }, 
    winrt::single_threaded_map<winrt::guid, winrt::Windows::Foundation::IInspectable>(
      std::unordered_map<winrt::guid, winrt::Windows::Foundation::IInspectable>
      {
        { winrt::Windows::Foundation::GuidHelper::CreateNewGuid(), bmp },
        { winrt::Windows::Foundation::GuidHelper::CreateNewGuid(), winrt::box_value(L"this is a test") },
        { winrt::Windows::Foundation::GuidHelper::CreateNewGuid(), bmp },
      }
      )).get();

  //std::wcout << "Q: " << question << L"\n";
  std::wcout << SetForegroundColor(RGB(0x28, 0x68, 0xff)) << L"\033[1m";
  std::wcout << "Here you go:\n\t";
  std::wcout << L"\033[0m"; 
  std::wcout << answer.Value() << L"\n";
  std::wcout << "Confidence: " << answer.Confidence() << L"\n";

  //auto answer = engine.AskAsync({ L"I need to find out who Olivia Wilde's boyfriend is and then calculate his age raised to the 0.23 power." }).get();

  /*
auto question = L"who is Olivia Wilde's boyfriend";

auto answer = engine.AskAsync({ question }).get();

*/


  return 0;

  //auto completionTask = openai.GetCompletionAsync(L"git clone ", L"text-davinci-003");
  //auto completions = completionTask.get();
  //for (auto const& c : completions) {
  //  std::wcout << c.Text() << L"\n";
  //}
  //std::wcout << L"\n\n---\n";
  auto completionTask2 = openaiEndpoint.GetCompletionAsync(
    winrt::OpenAI::builders::CompletionRequest{}
    .Prompt(L"git clone ")
    //.Model(L"text-davinci-003")
    .NCompletions(5)
    .Temperature(0.7f)
    .MaxTokens(100)
//    .Stream(true)
  );
  
  auto completions2 = completionTask2.get();
  auto i = 0;
  for (auto const& c : completions2) {
    std::wcout << L"Completion #" << i << L"\n";
    std::wcout << c.Text() << L"\n";
    std::wcout << (uint32_t)c.FinishReason() << L"\n";
    i++;
  }

  using namespace winrt::Windows::Foundation::Collections;
  using namespace winrt;


  auto promptTemplate = openaiEndpoint.CreateTemplate(L"Tell me a {adjective} joke about {content}");
  auto funnyJokeTask = promptTemplate.FormatAsync({ {L"adjective", L"funny"}, {L"content", L"chickens"} });
  auto funnyJoke = funnyJokeTask.get();

  std::wcout << L"\n\n" << funnyJoke << L"\n\n\n";

  auto example = openaiEndpoint.CreateFewShotTemplate({ L"word", L"antonym" });

  auto examples = std::vector {
    winrt::multi_threaded_map(std::unordered_map<hstring, hstring> { {L"word", L"happy"}, { L"antonym", L"sad" }}),
    winrt::multi_threaded_map(std::unordered_map<hstring, hstring>{ {L"word", L"tall"}, { L"antonym", L"short" }}),
    };
  example.Examples(std::move(examples));


  auto fewshot = example.ExecuteAsync(L"big").get();
  std::wcout << L"the opposite of big is " << fewshot.Lookup(L"antonym").begin() << L"\n";

  example = openaiEndpoint.CreateFewShotTemplate({ L"word", L"antonym", L"length" });
  examples = std::vector{
    winrt::multi_threaded_map(std::unordered_map<hstring, hstring> { {L"word", L"happy"}, { L"antonym", L"sad" }, { L"length", L"5" }}),
    winrt::multi_threaded_map(std::unordered_map<hstring, hstring>{ {L"word", L"tall"}, { L"antonym", L"short" }, { L"length", L"4"}}),
  };
  example.Examples(std::move(examples));
  auto word = L"huge";
  fewshot = example.ExecuteAsync(word).get();
  std::wcout << L"the opposite of " << word << L" is " << fewshot.Lookup(L"antonym").begin() << L" and the length is " << fewshot.Lookup(L"length").begin() << L"\n";

}

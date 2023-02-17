// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define NOMINMAX
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

#include <winrt/Windows.Web.Http.h>
#include <winrt/Windows.Data.Html.h>

using namespace winrt;
using namespace Windows::Data::Json;
namespace wfc = winrt::Windows::Foundation::Collections;
namespace wf = winrt::Windows::Foundation;
template<typename T> using async = wf::IAsyncOperation<T>;

winrt::OpenAI::SearchEndpoint searchEndpoint{ nullptr };
winrt::OpenAI::OpenAIClient openaiEndpoint{ nullptr };

#if 0
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



void DoSkillConnect() {
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

  std::wcout << SetForegroundColor(RGB(0xFF, 0xBA, 0x01)) << L"\xdb";
  std::wcout << SetForegroundColor(RGB(0x01, 0xA6, 0xF0)) << L"\xdb";
  std::wcout << L"\033[0m";
  std::wcout << L" " << std::wstring(12, (wchar_t)196) << L"\n\n";

  std::wcout << SetForegroundColor(RGB(0x28, 0x68, 0xff)) << L"\033[1m";
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
      winrt::OpenAI::Skill(L"GetTextFromBasketId", [](winrt::hstring expression, OpenAI::Context context, OpenAI::Engine engine) -> winrt::Windows::Foundation::IAsyncOperation<winrt::OpenAI::Answer> {
        auto input = co_await engine.ParseApiInputAsync(expression, single_threaded_vector<OpenAI::Parameter>(
    {
    OpenAI::Parameter{L"id", xaml_typename<winrt::guid>()}
    }));
  auto guid = winrt::guid{ input.GetNamedString(L"id") };
        auto value = context.Basket().Lookup(guid);
        auto str = winrt::unbox_value<winrt::hstring>(value);
        co_return OpenAI::Answer(str);
        })
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



  auto sf = winrt::Windows::Storage::KnownFolders::PicturesLibrary().GetFileAsync(L"IMG_20210726_164128.jpg").get();
  //auto sf = winrt::Windows::Storage::KnownFolders::PicturesLibrary().GetFileAsync(L"BlueStacks_ScreenShot.jpg").get();
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

}

void DoSimpleCompletion() {
  auto completionTask = openaiEndpoint.GetCompletionAsync(L"git clone ", L"text-davinci-003");
  auto completions = completionTask.get();
  for (auto const& c : completions) {
    std::wcout << c.Text() << L"\n";
  }
}

void DoCompletionRequestWithStreaming() {
  auto completionTask2 = openaiEndpoint.GetCompletionAsync(
    winrt::OpenAI::builders::CompletionRequest{}
    .Prompt(L"git clone ")
    //.Model(L"text-davinci-003")
    .NCompletions(5)
    .Temperature(0.7f)
    .MaxTokens(100)
    .Stream(true)
  );

  auto completions2 = completionTask2.get();
  auto i = 0;
  for (auto const& c : completions2) {
    std::wcout << L"Completion #" << i << L"\n";
    std::wcout << c.Text() << L"\n";
    std::wcout << (uint32_t)c.FinishReason() << L"\n";
    i++;
  }
}

  
void DoPromptTemplate() {
  auto promptTemplate = openaiEndpoint.CreateTemplate(L"Tell me a {adjective} joke about {content}");
  auto funnyJokeTask = promptTemplate.FormatAsync({ {L"adjective", L"funny"}, {L"content", L"chickens"} });
  auto funnyJoke = funnyJokeTask.get();

  std::wcout << L"\n\n" << funnyJoke << L"\n\n\n";
}

void DoFewShotTemplate() {
  auto example = openaiEndpoint.CreateFewShotTemplate({ L"word", L"antonym" });

  auto examples = std::vector{
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

#endif
#undef GetObject

void DoBingSearch() {
  auto search = OpenAI::SearchSkill(searchEndpoint);
  auto engine = winrt::OpenAI::builders::Engine{}
    .Skills({
      search,
      OpenAI::GPTSkill(openaiEndpoint)
      });
  engine.ConnectSkills();
  std::wstring question;
  std::wcout << "Bing web search x GPT -- Results inference\n\n";
  do {
    std::wcout << "?> ";
    std::getline(std::wcin, question);
    if (question == L"quit") break;
    auto result = search.ExecuteAsync(question, nullptr).get();
    std::wcout << std::vformat(LR"({{ "value": "{}", "confidence": {} }})", std::make_wformat_args(result.Value(), result.Confidence())) << L"\n";
  } while (true);
}

async<winrt::hstring> Fetch(wf::Uri url) {
  auto client = winrt::Windows::Web::Http::HttpClient{};
  auto response = co_await client.GetAsync(url);
  response.EnsureSuccessStatusCode();
  auto content = response.Content();
  auto str = co_await content.ReadAsStringAsync();
  co_return str;
}



std::wstring extractParagraphs(std::wstring_view html) {

  std::wstring output;
  std::wstring f_output;
  /*
  Using the position of each char on the string I created a for loop
  that loops to every char on the string. If a <p> is found everything
  is stored into the output. If a "</p>" is found the inner loop is broken

   */

  for (int i = 0; i < html.size(); i++) {
    if ((html[i] == L'<') && (html[i + 1] == L'p') && (html[i + 2] == L'>')) {
      for (int j = i; j < html.size(); j++) {
        output += html[j + 3];
        if ((html[j] == L'<') && (html[j + 1] == L'/') && (html[j + 2] == L'p')
          && (html[j + 3] == L'>')) {
          i = j + 4;
          output += L"\n\n";
          break;
        }
      }
    }
  }
  /* for larger html files my function will sometimes will include some "</p>",
  I created this extra forloop to get rid of them"
  */
  for (int i = 0; i < output.size(); i++) {
    if ((output[i] == L'<') && (output[i + 1] == L'/') && (output[i + 2] == L'p')
      && (output[i + 3] == L'>'))i += 4;

    f_output += output[i];
  }

  return f_output;
}

std::wstring removeTags(std::wstring htmltext) {

  std::wstring output;
  int flag = 0;
  /*
  Using the position of each char on the string array
  to check if its on the HTML tag range.
  if the char is not one of the tags, it is added to
  the output
  */
  for (int i = 0; i < htmltext.size(); i++) {
    if (htmltext[i] == L'>')
      flag = 0;
    if (htmltext[i] == '<')
      flag = 1;
    if (htmltext[i] != L'>' && !flag)
      output += htmltext[i];
  }

  return output;
}

std::wstring GetTextFromHtml(winrt::hstring html) {
  auto r = removeTags(extractParagraphs(html));
  return r;
}

void SkipWhitespace(std::wstring_view& contentStr)
{
  std::wstring_view whitespace{ L" \r\n\t" };
  while (contentStr.length() > 0 && whitespace.find_first_of(contentStr[0]) != std::wstring_view::npos) {
    auto firstNotSpace = contentStr.find_first_not_of(whitespace);
    contentStr = &contentStr[firstNotSpace];
  }
}

async<winrt::hstring> SummarizeTextByChunks(std::wstring_view text, const int& Chunk_Size_Bytes, const winrt::OpenAI::ISkill gpt, const winrt::OpenAI::Context& context)
{
  std::wstring summary;
  std::wstring_view contentStr = text;
  int i = 0;
  while (i < contentStr.size()) {
    const std::wstring_view start{ &contentStr[i] };

    auto target = i + std::min(Chunk_Size_Bytes, (int)start.length());
    std::wstring_view chunk;

    if (target > contentStr.size()) {
      chunk = std::wstring_view(&contentStr[i]);
      i = contentStr.size() + 1;
    } else {
      auto before_newline = contentStr.find_last_of('\n', target);
      auto before_period = contentStr.find_last_of('.', target);
      auto after_newline = start.find_first_of('\n', Chunk_Size_Bytes);
      auto after_period = start.find_first_of('.', Chunk_Size_Bytes);
      auto before_newline_count = target - before_newline;
      auto before_period_count = target - before_period;
      auto after_newline_count = after_newline - target;
      auto after_period_count = after_period - target;
      if (before_newline_count < before_period_count && before_newline > i) {
        chunk = std::wstring_view(&contentStr[i], before_newline - i + 1);
        i = before_newline + 1;
      } else if (before_period_count <= before_newline_count && before_period > i) {
        chunk = std::wstring_view(&contentStr[i], before_period - i + 1);
        i = before_period + 1;
      } else {
        chunk = std::wstring_view(&contentStr[i], Chunk_Size_Bytes);
        i += Chunk_Size_Bytes;
      }
    }

    auto summaryCompletion = co_await gpt.ExecuteAsync(L"Summarize this text in less than 2 sentences:\n" + chunk + L"\n", context);
    auto summaryText = summaryCompletion.Value();
    summary += summaryText;
  }
  co_return winrt::hstring{ summary };
}


async<OpenAI::Answer> WikipediaSearch(hstring expression, OpenAI::Context context, OpenAI::Engine engine) {
  auto searchJson = co_await searchEndpoint.SearchAsync(L"site:wikipedia.org " + expression);
  auto websites = searchJson.GetNamedObject(L"webPages");
  auto value = websites.GetNamedArray(L"value");
  winrt::hstring content;
  winrt::Windows::Foundation::Uri url{ nullptr };
  auto s = websites.Stringify();
  JsonArray sites;
  for (const auto& v_ : value) {
    auto v = v_.GetObject();
    auto urlStr = v.GetNamedString(L"url");
    auto nameStr = v.GetNamedString(L"name");
    auto snippetStr = v.GetNamedString(L"snippet");
    JsonObject site;
    site.Insert(L"name", JsonValue::CreateStringValue(nameStr));
    site.Insert(L"snippet", JsonValue::CreateStringValue(snippetStr));
    site.Insert(L"url", JsonValue::CreateStringValue(urlStr));
    sites.Append(site);
  }
  auto gpt = engine.GetSkill(L"openai");
  auto prompt = std::vformat(LR"(The following is a list of web results for the query "{}"\n\n{}\n\nReturn the url that corresponds to the most relevant entry\n\n)",
    std::make_wformat_args(expression, sites.Stringify()));
  auto bestLinkCompletion = co_await gpt.ExecuteAsync(prompt, context);
  auto urlStr = bestLinkCompletion.Value();

    try {
      url = wf::Uri{ urlStr };
      std::wstring host{ url.Host() };
      if (host.ends_with(L"wikipedia.org")) {
        content = co_await Fetch(url);
      }
    } catch (...){}
  
  if (content != L"") {
    auto text = GetTextFromHtml(content);
    std::wstring_view contentStr{ text };
    SkipWhitespace(contentStr);
    std::wstring_view loggedOutMessage{ L"Pages for logged out editors learn more" };
    if (contentStr.starts_with(loggedOutMessage)) {
      contentStr = contentStr.substr(loggedOutMessage.length());
    }
    SkipWhitespace(contentStr);

    
    constexpr auto Chunk_Size_Bytes = 1024;
    winrt::hstring summarizedChunks{ contentStr };
    while (summarizedChunks.size() > Chunk_Size_Bytes) {
      summarizedChunks = co_await SummarizeTextByChunks(summarizedChunks, Chunk_Size_Bytes, gpt, context);
    }
    
    auto overallCompletion = co_await gpt.ExecuteAsync(L"Summarize this text:\n" + summarizedChunks, context);
    auto summary = overallCompletion.Value();
    std::wstring_view alpha{ summary };
    auto first = alpha.find_first_not_of(L" \r\n\t");
    
    summary = alpha.substr(first);
    auto answer = OpenAI::Answer(summary);
    answer.Source(url);
    co_return answer;
  }
  co_return OpenAI::Answer(L"null");
}

void DoWikipedia() {
  auto search = OpenAI::Skill{ L"WikipediaResearch", & WikipediaSearch};
  auto engine = winrt::OpenAI::builders::Engine{}
    .Skills({
      search,
      OpenAI::GPTSkill(openaiEndpoint)
      });
  engine.ConnectSkills();
  std::wstring question;
  std::wcout << "Wikipedia research x GPT -- Results inference\n\n";
  do {
    std::wcout << "?> ";
    std::getline(std::wcin, question);
    if (question == L"quit") break;
    auto result = search.ExecuteAsync(question, nullptr).get();
    std::wcout << std::vformat(LR"({{ "value": "{}", "confidence": {}, "source": "{}" }})", std::make_wformat_args(result.Value(), result.Confidence(), result.Source().AbsoluteUri())) << L"\n";
  } while (true);
}

void DoEmbedding() {
  auto emTask = openaiEndpoint.GetEmbeddingAsync(L"the quick brown fox");
  auto emVector = emTask.get();
  std::array<double, 1024> embedding, other{ 1, 0 };
  emVector.GetMany(0, winrt::array_view(embedding));
  auto zero = winrt::OpenAI::EmbeddingUtils::EmbeddingDistance({ embedding.begin(), embedding.end() }, { embedding.begin(), embedding.end() }, winrt::OpenAI::Similarity::L2);
  auto d = winrt::OpenAI::EmbeddingUtils::EmbeddingDistance({ embedding.begin(), embedding.end() }, { other.begin(), other.end() });

}

int main()
{
  winrt::init_apartment(/*winrt::apartment_type::multi_threaded*/);

  searchEndpoint = winrt::OpenAI::builders::SearchEndpoint();

  openaiEndpoint = winrt::OpenAI::builders::OpenAIClient()
    .CompletionUri(winrt::Windows::Foundation::Uri{ L"https://lrsopenai.openai.azure.com/openai/deployments/Text-Davinci3-Deployment/completions?api-version=2022-12-01" })
    .UseBearerTokenAuthorization(false)
    ;

  DoWikipedia();
  //DoBingSearch();
  

  //DoSkillConnect();
}
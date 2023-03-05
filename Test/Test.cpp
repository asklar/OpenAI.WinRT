#define NOMINMAX
#include <iostream>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>

#include <winrt/builders/OpenAI.h>
#include <winrt/builders/helpers.h>

#undef GetObject


using namespace winrt;
namespace wfc = winrt::Windows::Foundation::Collections;
namespace wf = winrt::Windows::Foundation;
template<typename T> using async = wf::IAsyncOperation<T>;

winrt::OpenAI::OpenAIClient openaiEndpoint{ nullptr };


void DoSimpleCompletion() {
  auto completionTask = openaiEndpoint.GetCompletionAsync(L"git clone ", L"text-davinci-003");
  auto completions = completionTask.get();
  for (auto const& c : completions) {
    std::wcout << c.Text() << L"\n";
  }
}

void DoGpt35TurboCompletion() {
  auto completionTask = openaiEndpoint.GetCompletionAsync(L"git clone ", L"gpt-3.5-turbo");
  auto completions = completionTask.get();
  for (auto const& c : completions) {
    std::wcout << c.Text() << L"\n";
  }
}

void DoChat() {
  auto chatTask = openaiEndpoint.GetChatResponseAsync(
    winrt::OpenAI::builders::ChatRequest{}
    .Stream(true)
    .Messages({
      winrt::OpenAI::ChatMessage(winrt::OpenAI::ChatRole::System, L"Reply in spanish"),
      winrt::OpenAI::ChatMessage(winrt::OpenAI::ChatRole::User, L"What is the meaning of the golden ratio?"),
      }
  )).get();
  for (const auto& c : chatTask) {
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



void DoEmbedding() {
  auto emTask = openaiEndpoint.GetEmbeddingAsync(L"the quick brown fox");
  auto emVector = emTask.get();
  std::array<double, 1024> embedding, other{ 1, 0 };
  emVector.GetMany(0, winrt::array_view(embedding));
  auto zero = winrt::OpenAI::EmbeddingUtils::EmbeddingDistance(emVector.GetView(), emVector.GetView(), winrt::OpenAI::Similarity::L2);
  auto d = winrt::OpenAI::EmbeddingUtils::EmbeddingDistance(emVector.GetView(), { other.begin(), other.end() });

}

int main()
{
  winrt::init_apartment(/*winrt::apartment_type::multi_threaded*/);

  openaiEndpoint = winrt::OpenAI::builders::OpenAIClient()
    .ApiKey(L"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");

  DoGpt35TurboCompletion();

  DoChat();

//  DoSimpleCompletion();

//  DoEmbedding();
}
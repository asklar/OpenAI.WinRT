// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/openai.h>
#include <winrt/builders/OpenAI.CompletionRequest.h>

int main()
{
  winrt::init_apartment(/*winrt::apartment_type::multi_threaded*/);
  auto openai = winrt::OpenAI::OpenAIClient{};
  //auto completionTask = openai.GetCompletionAsync(L"git clone ", L"text-davinci-003");
  //auto completions = completionTask.get();
  //for (auto const& c : completions) {
  //  std::wcout << c.Text() << L"\n";
  //}
  //std::wcout << L"\n\n---\n";
  auto completionTask2 = openai.GetCompletionAsync(
    winrt::OpenAI::builders::CompletionRequest{}
      .Prompt(L"git clone ")
      .Model(L"text-davinci-003")
      .NCompletions(5)
      .Temperature(0.7f)
      .MaxTokens(100)
  );
  auto completions2 = completionTask2.get();
  auto i = 0;
  for (auto const& c : completions2) {
    std::wcout << L"Completion #" << i << L"\n";
    std::wcout << c.Text() << L"\n";
    i++;
  }
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

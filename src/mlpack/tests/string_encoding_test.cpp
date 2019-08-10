/**
 * @file string_encoding_test.cpp
 * @author Jeffin Sam
 *
 * Tests for the StringEncoding class.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#include <mlpack/core.hpp>
#include <mlpack/core/boost_backport/boost_backport_string_view.hpp>
#include <mlpack/core/data/tokenizers/split_by_any_of.hpp>
#include <mlpack/core/data/tokenizers/char_extract.hpp>
#include <mlpack/core/data/string_encoding.hpp>
#include <mlpack/core/data/string_encoding_policies/dictionary_encoding_policy.hpp>
#include <mlpack/core/data/string_encoding_policies/bag_of_words_encoding_policy.hpp>
#include <mlpack/core/data/string_encoding_policies/tf_idf_encoding_policy.hpp>
#include <boost/test/unit_test.hpp>
#include <memory>
#include "test_tools.hpp"
#include "serialization.hpp"

using namespace mlpack;
using namespace mlpack::data;
using namespace std;

BOOST_AUTO_TEST_SUITE(StringEncodingTest);

//! Common input for some tests.
static vector<string> stringEncodingInput = {
    "mlpack is an intuitive, fast, and flexible C++ machine learning library "
    "with bindings to other languages. ",
    "It is meant to be a machine learning analog to LAPACK, and aims to "
    "implement a wide array of machine learning methods and functions "
    "as a \"swiss army knife\" for machine learning researchers.",
    "In addition to its powerful C++ interface, mlpack also provides "
    "command-line programs and Python bindings."
};

/**
 * Test the dictionary encoding algorithm.
 */
BOOST_AUTO_TEST_CASE(DictionaryEncodingTest)
{
  using DictionaryType = StringEncodingDictionary<boost::string_view>;

  arma::mat output;
  DictionaryEncoding<SplitByAnyOf::TokenType> encoder;
  SplitByAnyOf tokenizer(" .,\"");

  encoder.Encode(stringEncodingInput, output, tokenizer);

  const DictionaryType& dictionary = encoder.Dictionary();

  // Checking that everything is mapped to different numbers
  std::unordered_map<size_t, size_t> keysCount;
  for (auto& keyValue : dictionary.Mapping())
  {
    keysCount[keyValue.second]++;
    // Every token should be mapped only once
    BOOST_REQUIRE_EQUAL(keysCount[keyValue.second], 1);
  }

  arma::mat expected = {
    {  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,  0,
       0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
    { 17,  2, 18, 14, 19, 20,  9, 10, 21, 14, 22,  6, 23, 14, 24, 20, 25,
      26, 27,  9, 10, 28,  6, 29, 30, 20, 31, 32, 33, 34,  9, 10, 35 },
    { 36, 37, 14, 38, 39,  8, 40,  1, 41, 42, 43, 44,  6, 45, 13,  0,  0,
       0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }
  };

  CheckMatrices(output, expected);
}

/**
 * Test the one pass modification of the dictionary encoding algorithm.
 */
BOOST_AUTO_TEST_CASE(OnePassDictionaryEncodingTest)
{
  using DictionaryType = StringEncodingDictionary<boost::string_view>;

  vector<vector<size_t>> output;
  DictionaryEncoding<SplitByAnyOf::TokenType> encoder(
      (DictionaryEncodingPolicy()));
  SplitByAnyOf tokenizer(" .,\"");

  encoder.Encode(stringEncodingInput, output, tokenizer);

  const DictionaryType& dictionary = encoder.Dictionary();

  // Checking that everything is mapped to different numbers
  std::unordered_map<size_t, size_t> keysCount;
  for (auto& keyValue : dictionary.Mapping())
  {
    keysCount[keyValue.second]++;
    // Every token should be mapped only once
    BOOST_REQUIRE_EQUAL(keysCount[keyValue.second], 1);
  }

  vector<vector<size_t>> expected = {
    {  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16 },
    { 17,  2, 18, 14, 19, 20,  9, 10, 21, 14, 22,  6, 23, 14, 24, 20, 25,
      26, 27,  9, 10, 28,  6, 29, 30, 20, 31, 32, 33, 34,  9, 10, 35 },
    { 36, 37, 14, 38, 39,  8, 40,  1, 41, 42, 43, 44,  6, 45, 13 }
  };

  BOOST_REQUIRE(output == expected);
}


/**
 * Test for the SplitByAnyOf tokenizer.
 */
BOOST_AUTO_TEST_CASE(SplitByAnyOfTokenizerTest)
{
  std::vector<boost::string_view> tokens;
  boost::string_view line(stringEncodingInput[0]);
  SplitByAnyOf tokenizer(" ,.");
  boost::string_view token = tokenizer(line);

  while (!token.empty())
  {
    tokens.push_back(token);
    token = tokenizer(line);
  }

  vector<string> expected = { "mlpack", "is", "an", "intuitive", "fast",
    "and", "flexible", "C++", "machine", "learning", "library", "with",
    "bindings", "to", "other", "languages"
  };

  BOOST_REQUIRE_EQUAL(tokens.size(), expected.size());

  for (size_t i = 0; i < tokens.size(); i++)
    BOOST_REQUIRE_EQUAL(tokens[i], expected[i]);
}

/**
* Test Dictionary encoding for characters using lamda function.
*/
BOOST_AUTO_TEST_CASE(DictionaryEncodingIndividualCharactersTest)
{
  vector<string> input = {
    "GACCA",
    "ABCABCD",
    "GAB"
  };

  arma::mat output;
  DictionaryEncoding<CharExtract::TokenType> encoder;

  // Passing a empty string to encode characters
  encoder.Encode(input, output, CharExtract());

  arma::mat target = {
    { 1, 2, 3, 3, 2, 0, 0 },
    { 2, 4, 3, 2, 4, 3, 5 },
    { 1, 2, 4, 0, 0, 0, 0 }
  };
  CheckMatrices(output, target);
}

/**
 * Test the one pass modification of the dictionary encoding algorithm
 * in case of individual character encoding.
 */
BOOST_AUTO_TEST_CASE(OnePassDictionaryEncodingIndividualCharactersTest)
{
  std::vector<string> input = {
    "GACCA",
    "ABCABCD",
    "GAB"
  };

  vector<vector<size_t>> output;
  DictionaryEncoding<CharExtract::TokenType> encoder;

  // Passing a empty string to encode characters
  encoder.Encode(input, output, CharExtract());

  vector<vector<size_t>> expected = {
    { 1, 2, 3, 3, 2 },
    { 2, 4, 3, 2, 4, 3, 5 },
    { 1, 2, 4 }
  };

  BOOST_REQUIRE(output == expected);
}

/**
 * Test the functionality of copy constructor.
 */
BOOST_AUTO_TEST_CASE(StringEncodingCopyTest)
{
  using DictionaryType = StringEncodingDictionary<boost::string_view>;
  arma::sp_mat output;
  DictionaryEncoding<SplitByAnyOf::TokenType> encoderCopy;
  SplitByAnyOf tokenizer(" ,.");

  vector<pair<string, size_t>> naiveDictionary;

  {
    DictionaryEncoding<SplitByAnyOf::TokenType> encoder;
    encoder.Encode(stringEncodingInput, output, tokenizer);

    for (const string& token : encoder.Dictionary().Tokens())
    {
      naiveDictionary.emplace_back(token, encoder.Dictionary().Value(token));
    }

    encoderCopy = DictionaryEncoding<SplitByAnyOf::TokenType>(encoder);
  }

  const DictionaryType& copiedDictionary = encoderCopy.Dictionary();

  BOOST_REQUIRE_EQUAL(naiveDictionary.size(), copiedDictionary.Size());

  for (const pair<string, size_t>& keyValue : naiveDictionary)
  {
    BOOST_REQUIRE(copiedDictionary.HasToken(keyValue.first));
    BOOST_REQUIRE_EQUAL(copiedDictionary.Value(keyValue.first),
        keyValue.second);
  }
}

/**
 * Test the move assignment operator.
 */
BOOST_AUTO_TEST_CASE(StringEncodingMoveTest)
{
  using DictionaryType = StringEncodingDictionary<boost::string_view>;
  arma::sp_mat output;
  DictionaryEncoding<SplitByAnyOf::TokenType> encoderCopy;
  SplitByAnyOf tokenizer(" ,.");

  vector<pair<string, size_t>> naiveDictionary;

  {
    DictionaryEncoding<SplitByAnyOf::TokenType> encoder;
    encoder.Encode(stringEncodingInput, output, tokenizer);

    for (const string& token : encoder.Dictionary().Tokens())
    {
      naiveDictionary.emplace_back(token, encoder.Dictionary().Value(token));
    }

    encoderCopy = std::move(encoder);
  }

  const DictionaryType& copiedDictionary = encoderCopy.Dictionary();

  BOOST_REQUIRE_EQUAL(naiveDictionary.size(), copiedDictionary.Size());

  for (const pair<string, size_t>& keyValue : naiveDictionary)
  {
    BOOST_REQUIRE(copiedDictionary.HasToken(keyValue.first));
    BOOST_REQUIRE_EQUAL(copiedDictionary.Value(keyValue.first),
        keyValue.second);
  }
}

/**
 * The function checks that the given dictionaries contain the same data.
 */
template<typename TokenType>
void CheckDictionaries(const StringEncodingDictionary<TokenType>& expected,
                       const StringEncodingDictionary<TokenType>& obtained)
{
  // MapType is equal to std::unordered_map<Token, size_t>.
  using MapType = typename StringEncodingDictionary<TokenType>::MapType;

  const MapType& mapping = obtained.Mapping();
  const MapType& expectedMapping = expected.Mapping();

  BOOST_REQUIRE_EQUAL(mapping.size(), expectedMapping.size());

  for (auto& keyVal : expectedMapping)
  {
    BOOST_REQUIRE_EQUAL(mapping.at(keyVal.first), keyVal.second);
  }

  for (auto& keyVal : mapping)
  {
    BOOST_REQUIRE_EQUAL(expectedMapping.at(keyVal.first), keyVal.second);
  }
}

/**
 * This is a specialization of the CheckDictionaries() function for
 * the boost::string_view token type.
 */
template<>
void CheckDictionaries(
    const StringEncodingDictionary<boost::string_view>& expected,
    const StringEncodingDictionary<boost::string_view>& obtained)
{
  /* MapType is equal to
   *
   * std::unordered_map<boost::string_view,
   *                    size_t,
   *                    boost::hash<boost::string_view>>.
   */
  using MapType =
      typename StringEncodingDictionary<boost::string_view>::MapType;

  const std::deque<std::string>& expectedTokens = expected.Tokens();
  const std::deque<std::string>& tokens = obtained.Tokens();
  const MapType& expectedMapping = expected.Mapping();
  const MapType& mapping = obtained.Mapping();

  BOOST_REQUIRE_EQUAL(tokens.size(), expectedTokens.size());
  BOOST_REQUIRE_EQUAL(mapping.size(), expectedMapping.size());
  BOOST_REQUIRE_EQUAL(mapping.size(), tokens.size());

  for (size_t i = 0; i < tokens.size(); i++)
  {
    BOOST_REQUIRE_EQUAL(tokens[i], expectedTokens[i]);
    BOOST_REQUIRE_EQUAL(expectedMapping.at(tokens[i]), mapping.at(tokens[i]));
  }
}

/**
 * This is a specialization of the CheckDictionaries() function for
 * the integer token type.
 */
template<>
void CheckDictionaries(const StringEncodingDictionary<int>& expected,
                       const StringEncodingDictionary<int>& obtained)
{
  // MapType is equal to std::arry<size_t, 256>.
  using MapType = typename StringEncodingDictionary<int>::MapType;

  const MapType& expectedMapping = expected.Mapping();
  const MapType& mapping = obtained.Mapping();

  for (size_t i = 0; i < mapping.size(); i++)
  {
    BOOST_REQUIRE_EQUAL(mapping[i], expectedMapping[i]);
  }
}

/**
 * Serialization test for the dictionary encoding algorithm with
 * the SplitByAnyOf tokenizer.
 */
BOOST_AUTO_TEST_CASE(SplitByAnyOfDictionaryEncodingSerialization)
{
  using EncoderType = DictionaryEncoding<SplitByAnyOf::TokenType>;

  EncoderType encoder;
  SplitByAnyOf tokenizer(" ,.");
  arma::mat output;

  encoder.Encode(stringEncodingInput, output, tokenizer);

  EncoderType xmlEncoder, textEncoder, binaryEncoder;
  arma::mat xmlOutput, textOutput, binaryOutput;

  SerializeObjectAll(encoder, xmlEncoder, textEncoder, binaryEncoder);

  CheckDictionaries(encoder.Dictionary(), xmlEncoder.Dictionary());
  CheckDictionaries(encoder.Dictionary(), textEncoder.Dictionary());
  CheckDictionaries(encoder.Dictionary(), binaryEncoder.Dictionary());

  xmlEncoder.Encode(stringEncodingInput, xmlOutput, tokenizer);
  textEncoder.Encode(stringEncodingInput, textOutput, tokenizer);
  binaryEncoder.Encode(stringEncodingInput, binaryOutput, tokenizer);

  CheckMatrices(output, xmlOutput, textOutput, binaryOutput);
}

/**
 * Serialization test for the Bag Of Words encoding algorithm with
 * the CharExtract tokenizer.
 */
BOOST_AUTO_TEST_CASE(CharExtractBagOfWordsEncodingSerialization)
{
  using EncoderType = BagOfWordsEncoding<CharExtract::TokenType>;

  EncoderType encoder;
  CharExtract tokenizer;
  arma::mat output;
  encoder.Encode(stringEncodingInput, output, tokenizer);

  EncoderType xmlEncoder, textEncoder, binaryEncoder;
  arma::mat xmlOutput, textOutput, binaryOutput;

  SerializeObjectAll(encoder, xmlEncoder, textEncoder, binaryEncoder);

  CheckDictionaries(encoder.Dictionary(), xmlEncoder.Dictionary());
  CheckDictionaries(encoder.Dictionary(), textEncoder.Dictionary());
  CheckDictionaries(encoder.Dictionary(), binaryEncoder.Dictionary());

  xmlEncoder.Encode(stringEncodingInput, xmlOutput, tokenizer);
  textEncoder.Encode(stringEncodingInput, textOutput, tokenizer);
  binaryEncoder.Encode(stringEncodingInput, binaryOutput, tokenizer);

  CheckMatrices(output, xmlOutput, textOutput, binaryOutput);
}

BOOST_AUTO_TEST_CASE(BagOfWordsEncodingTest)
{
  using DictionaryType = StringEncodingDictionary<boost::string_view>;

  arma::mat output;
  BagOfWordsEncoding<SplitByAnyOf::TokenType> encoder;
  SplitByAnyOf tokenizer(" ");

  encoder.Encode(stringEncodingInput, output, tokenizer);
  const DictionaryType& dictionary = encoder.Dictionary();

  // Checking that everything is mapped to different numbers
  std::unordered_map<size_t, size_t> keysCount;
  for (auto& keyValue : dictionary.Mapping())
  {
    keysCount[keyValue.second]++;
    // Every token should be mapped only once
    BOOST_REQUIRE_EQUAL(keysCount[keyValue.second], 1);
  }
  arma::mat expected = {
    {  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  },
    {  0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  },
    {  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1  }
  };
  CheckMatrices(output, expected);
}

*
 * Test the one pass modification of the Bag of Words encoding algorithm.
 
BOOST_AUTO_TEST_CASE(OnePassBagOfWordsEncodingTest)
{
  using DictionaryType = StringEncodingDictionary<boost::string_view>;

  vector<vector<size_t>> output;
  BagOfWordsEncoding<SplitByAnyOf::TokenType> encoder(
      (BagOfWordsEncodingPolicy()));
  SplitByAnyOf tokenizer(" ");

  encoder.Encode(stringEncodingInput, output, tokenizer);

  const DictionaryType& dictionary = encoder.Dictionary();

  // Checking that everything is mapped to different numbers
  std::unordered_map<size_t, size_t> keysCount;
  for (auto& keyValue : dictionary.Mapping())
  {
    keysCount[keyValue.second]++;
    // Every token should be mapped only once
    BOOST_REQUIRE_EQUAL(keysCount[keyValue.second], 1);
  }

  vector<vector<size_t>> expected = {
    {  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  },
    {  0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  },
    {  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1  }
  };

  BOOST_REQUIRE(output == expected);
}

/**
* Test Bag of Words encoding for characters using lamda function.
*/
BOOST_AUTO_TEST_CASE(BagOfWordsEncodingIndividualCharactersTest)
{
  vector<string> input = {
    "GACCA",
    "ABCABCD",
    "GAB"
  };

  arma::mat output;
  BagOfWordsEncoding<CharExtract::TokenType> encoder;

  // Passing a empty string to encode characters
  encoder.Encode(input, output, CharExtract());
  arma::mat target = {
    { 1, 1, 1, 0, 0 },
    { 0, 1, 1, 1, 1 },
    { 1, 1, 0, 1, 0 }
  };

  CheckMatrices(output, target);
}

/**
 * Test the one pass modification of the Bag of Words encoding algorithm
 * in case of individual character encoding.
 */
BOOST_AUTO_TEST_CASE(OnePassBagOfWordsEncodingIndividualCharactersTest)
{
  std::vector<string> input = {
    "GACCA",
    "ABCABCD",
    "GAB"
  };

  vector<vector<size_t>> output;
  BagOfWordsEncoding<CharExtract::TokenType> encoder;

  // Passing a empty string to encode characters
  encoder.Encode(input, output, CharExtract());

  vector<vector<size_t>> expected = {
    { 1, 1, 1, 0, 0 },
    { 0, 1, 1, 1, 1 },
    { 1, 1, 0, 1, 0 }
  };

  BOOST_REQUIRE(output == expected);
}

// /**
//  * Test the Tf-Idf Encoding using rawcount type and smoothidf as true,
//  * which is the deafult values used for algorithim.
//  */
// BOOST_AUTO_TEST_CASE(RawCountSmoothIdfEncodingTest)
// {
//   using DictionaryType = StringEncodingDictionary<boost::string_view>;

//   arma::mat output;
//   TfIdfEncoding<SplitByAnyOf::TokenType> encoder;
//   SplitByAnyOf tokenizer(" ");

//   encoder.Encode(stringEncodingInput, output, tokenizer);

//   const DictionaryType& dictionary = encoder.Dictionary();

//   // Checking that everything is mapped to different numbers
//   std::unordered_map<size_t, size_t> keysCount;
//   for (auto& keyValue : dictionary.Mapping())
//   {
//     keysCount[keyValue.second]++;
//     // Every token should be mapped only once
//     BOOST_REQUIRE_EQUAL(keysCount[keyValue.second], 1);
//   }
//   arma::mat expected = {
//     {  1.28768207245178, 1.28768207245178, 1.69314718055995, 1.69314718055995,
//        1.69314718055995, 1, 1.69314718055995, 1.28768207245178,
//        1.28768207245178, 1.28768207245178, 1.69314718055995, 1.69314718055995,
//        1.69314718055995, 1, 1.69314718055995, 1.69314718055995, 0, 0, 0, 0, 0,
//        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//        0  },
//     {  0, 1.28768207245178, 0, 0, 0, 2, 0, 0, 3.86304621735534,
//        3.86304621735534, 0, 0, 0, 3, 0, 0, 1.69314718055995, 1.69314718055995,
//        1.69314718055995, 5.07944154167984, 1.69314718055995, 1.69314718055995,
//        1.69314718055995, 1.69314718055995, 1.69314718055995, 1.69314718055995,
//        1.69314718055995, 1.69314718055995, 1.69314718055995, 1.69314718055995,
//        1.69314718055995, 1.69314718055995, 1.69314718055995, 1.69314718055995,
//        1.69314718055995, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  },
//     {  1.28768207245178, 0, 0, 0, 0, 1, 0, 1.28768207245178, 0, 0, 0, 0, 0, 1,
//        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//        1.69314718055995, 1.69314718055995, 1.69314718055995, 1.69314718055995,
//        1.69314718055995, 1.69314718055995, 1.69314718055995, 1.69314718055995,
//        1.69314718055995, 1.69314718055995, 1.69314718055995  }
//   };
//   CheckMatrices(output, expected, 1e-12);
// }

// /**
//  * Test the one pass modification of the TfIdf encoding algorithm, using rawcount
//  * as type of tf and smoothIdf as true.
//  */
// BOOST_AUTO_TEST_CASE(OnePassRawCountSmoothIdfEncodingTest)
// {
//   using DictionaryType = StringEncodingDictionary<boost::string_view>;

//   vector<vector<double>> output;
//   TfIdfEncoding<SplitByAnyOf::TokenType> encoder(
//       (TfIdfEncodingPolicy()));
//   SplitByAnyOf tokenizer(" ");

//   encoder.Encode(stringEncodingInput, output, tokenizer);

//   const DictionaryType& dictionary = encoder.Dictionary();

//   // Checking that everything is mapped to different numbers
//   std::unordered_map<size_t, size_t> keysCount;
//   for (auto& keyValue : dictionary.Mapping())
//   {
//     keysCount[keyValue.second]++;
//     // Every token should be mapped only once
//     BOOST_REQUIRE_EQUAL(keysCount[keyValue.second], 1);
//   }

//   vector<vector<double>> expected = {
//     {  1.28768207245178, 1.28768207245178, 1.69314718055995, 1.69314718055995,
//        1.69314718055995, 1, 1.69314718055995, 1.28768207245178,
//        1.28768207245178, 1.28768207245178, 1.69314718055995, 1.69314718055995,
//        1.69314718055995, 1, 1.69314718055995, 1.69314718055995, 0, 0, 0, 0, 0,
//        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//        0  },
//     {  0, 1.28768207245178, 0, 0, 0, 2, 0, 0, 3.86304621735534,
//        3.86304621735534, 0, 0, 0, 3, 0, 0, 1.69314718055995, 1.69314718055995,
//        1.69314718055995, 5.07944154167984, 1.69314718055995, 1.69314718055995,
//        1.69314718055995, 1.69314718055995, 1.69314718055995, 1.69314718055995,
//        1.69314718055995, 1.69314718055995, 1.69314718055995, 1.69314718055995,
//        1.69314718055995, 1.69314718055995, 1.69314718055995, 1.69314718055995,
//        1.69314718055995, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  },
//     {  1.28768207245178, 0, 0, 0, 0, 1, 0, 1.28768207245178, 0, 0, 0, 0, 0, 1,
//        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//        1.69314718055995, 1.69314718055995, 1.69314718055995, 1.69314718055995,
//        1.69314718055995, 1.69314718055995, 1.69314718055995, 1.69314718055995,
//        1.69314718055995, 1.69314718055995, 1.69314718055995  }
//   };
//   for (size_t i = 0; i < expected.size(); i++)
//     for (size_t j = 0; j < expected[i].size(); j++)
//       BOOST_REQUIRE_CLOSE(expected[i][j], output[i][j], 1e-12);
// }

// /**
//  * Test TFIDF encoding for characters using lamda function, using rawcount as tf
//  * type and smoothidf as true.
//  */
// BOOST_AUTO_TEST_CASE(RawCountSmoothIdfEncodingIndividualCharactersTest)
// {
//   vector<string> input = {
//     "GACCA",
//     "ABCABCD",
//     "GAB"
//   };

//   arma::mat output;
//   TfIdfEncoding<CharExtract::TokenType> encoder;

//   // Passing a empty string to encode charactersrawcountsmoothidftrue
//   encoder.Encode(input, output, CharExtract());
//   arma::mat target = {
//     { 1.2876820724517808, 2, 2.5753641449035616, 0, 0 },
//     { 0, 2, 2.5753641449035616, 2.5753641449035616, 1.6931471805599454 },
//     { 1.2876820724517808, 1, 0, 1.2876820724517808, 0 }
//   };
//   CheckMatrices(output, target, 1e-12);
// }

// /**
//  * Test the one pass modification of the Tf-Idf encoding algorithm
//  * in case of individual character encoding using default values.
//  */
// BOOST_AUTO_TEST_CASE(OnePassRawCountSmoothIdfEncodingIndividualCharactersTest)
// {
//   std::vector<string> input = {
//     "GACCA",
//     "ABCABCD",
//     "GAB"
//   };

//   vector<vector<double>> output;
//   TfIdfEncoding<CharExtract::TokenType> encoder;

//   // Passing a empty string to encode characters
//   encoder.Encode(input, output, CharExtract());
//   vector<vector<double>> expected = {
//     { 1.2876820724517808, 2, 2.5753641449035616, 0, 0 },
//     { 0, 2, 2.5753641449035616, 2.5753641449035616, 1.6931471805599454 },
//     { 1.2876820724517808, 1, 0, 1.2876820724517808, 0 }
//   };
//   for (size_t i = 0; i < expected.size(); i++)
//     for (size_t j = 0; j < expected[i].size(); j++)
//       BOOST_REQUIRE_CLOSE(expected[i][j], output[i][j], 1e-12);
// }

// /**
//  * Test the Tf-Idf Encoding using rawcount type and smoothidf as false.
//  */
// BOOST_AUTO_TEST_CASE(TfIdfRawCountEncodingTest)
// {
//   using DictionaryType = StringEncodingDictionary<boost::string_view>;

//   arma::mat output;
//   TfIdfEncoding<SplitByAnyOf::TokenType> encoder(
//       (TfIdfEncodingPolicy(0, false)));
//   SplitByAnyOf tokenizer(" ");

//   encoder.Encode(stringEncodingInput, output, tokenizer);

//   const DictionaryType& dictionary = encoder.Dictionary();

//   // Checking that everything is mapped to different numbers
//   std::unordered_map<size_t, size_t> keysCount;
//   for (auto& keyValue : dictionary.Mapping())
//   {
//     keysCount[keyValue.second]++;
//     // Every token should be mapped only once
//     BOOST_REQUIRE_EQUAL(keysCount[keyValue.second], 1);
//   }

//   arma::mat expected = {
//     {  1.40546510810816, 1.40546510810816, 2.09861228866811, 2.09861228866811,
//        2.09861228866811, 1, 2.09861228866811, 1.40546510810816,
//        1.40546510810816, 1.40546510810816, 2.09861228866811, 2.09861228866811,
//        2.09861228866811, 1, 2.09861228866811, 2.09861228866811, 0, 0, 0, 0, 0,
//        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//        0  },
//     {  0, 1.40546510810816, 0, 0, 0, 2, 0, 0, 4.21639532432449,
//        4.21639532432449, 0, 0, 0, 3, 0, 0, 2.09861228866811, 2.09861228866811,
//        2.09861228866811, 6.29583686600433, 2.09861228866811, 2.09861228866811,
//        2.09861228866811, 2.09861228866811, 2.09861228866811, 2.09861228866811,
//        2.09861228866811, 2.09861228866811, 2.09861228866811, 2.09861228866811,
//        2.09861228866811, 2.09861228866811, 2.09861228866811, 2.09861228866811,
//        2.09861228866811, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  },
//     {  1.40546510810816, 0, 0, 0, 0, 1, 0, 1.40546510810816, 0, 0, 0, 0, 0, 1,
//        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//        2.09861228866811, 2.09861228866811, 2.09861228866811, 2.09861228866811,
//        2.09861228866811, 2.09861228866811, 2.09861228866811, 2.09861228866811,
//        2.09861228866811, 2.09861228866811, 2.09861228866811  }
//   };
//   CheckMatrices(output, expected, 1e-12);
// }

// *
//  * Test the one pass modification of the TfIdf encoding algorithm, with rawcount
//  * as type, but with smoothidf as false.
 
// BOOST_AUTO_TEST_CASE(OnePassTfIdfRawCountEncodingTest)
// {
//   using DictionaryType = StringEncodingDictionary<boost::string_view>;

//   vector<vector<double>> output;
//   TfIdfEncoding<SplitByAnyOf::TokenType> encoder(0, false);
//   SplitByAnyOf tokenizer(" ");

//   encoder.Encode(stringEncodingInput, output, tokenizer);

//   const DictionaryType& dictionary = encoder.Dictionary();

//   // Checking that everything is mapped to different numbers
//   std::unordered_map<size_t, size_t> keysCount;
//   for (auto& keyValue : dictionary.Mapping())
//   {
//     keysCount[keyValue.second]++;
//     // Every token should be mapped only once
//     BOOST_REQUIRE_EQUAL(keysCount[keyValue.second], 1);
//   }

//   vector<vector<double>> expected = {
//     {  1.40546510810816, 1.40546510810816, 2.09861228866811, 2.09861228866811,
//        2.09861228866811, 1, 2.09861228866811, 1.40546510810816,
//        1.40546510810816, 1.40546510810816, 2.09861228866811, 2.09861228866811,
//        2.09861228866811, 1, 2.09861228866811, 2.09861228866811, 0, 0, 0, 0, 0,
//        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//        0  },
//     {  0, 1.40546510810816, 0, 0, 0, 2, 0, 0, 4.21639532432449,
//        4.21639532432449, 0, 0, 0, 3, 0, 0, 2.09861228866811, 2.09861228866811,
//        2.09861228866811, 6.29583686600433, 2.09861228866811, 2.09861228866811,
//        2.09861228866811, 2.09861228866811, 2.09861228866811, 2.09861228866811,
//        2.09861228866811, 2.09861228866811, 2.09861228866811, 2.09861228866811,
//        2.09861228866811, 2.09861228866811, 2.09861228866811, 2.09861228866811,
//        2.09861228866811, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  },
//     {  1.40546510810816, 0, 0, 0, 0, 1, 0, 1.40546510810816, 0, 0, 0, 0, 0, 1,
//        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//        2.09861228866811, 2.09861228866811, 2.09861228866811, 2.09861228866811,
//        2.09861228866811, 2.09861228866811, 2.09861228866811, 2.09861228866811,
//        2.09861228866811, 2.09861228866811, 2.09861228866811  }
//   };
//   for (size_t i = 0; i < expected.size(); i++)
//     for (size_t j = 0; j < expected[i].size(); j++)
//       BOOST_REQUIRE_CLOSE(expected[i][j], output[i][j], 1e-12);
// }

// /**
//  * Test TFIDF encoding for characters using lamda function, using rawcount as
//  * tf type and smoothidf as false.
//  */
// BOOST_AUTO_TEST_CASE(RawcountTfIdfEncodingIndividualCharactersTest)
// {
//   vector<string> input = {
//     "GACCA",
//     "ABCABCD",
//     "GAB"
//   };

//   arma::mat output;
//   TfIdfEncoding<CharExtract::TokenType> encoder(0, false);

//   // Passing a empty string to encode charactersrawcountsmoothidftrue
//   encoder.Encode(input, output, CharExtract());
//   arma::mat target = {
//     { 1.4054651081081644, 2, 2.8109302162163288, 0, 0 },
//     { 0, 2, 2.8109302162163288, 2.8109302162163288, 2.0986122886681100 },
//     { 1.4054651081081644, 1, 0, 1.4054651081081644, 0 }
//   };
//   CheckMatrices(output, target, 1e-12);
// }

// /**
//  * Test the one pass modification of the Tf Idf encoding algorithm
//  * in case of individual character encoding, using raw count as type,
//  * and smoothidf as false.
//  */
// BOOST_AUTO_TEST_CASE(OnePassRawcountEncodingIndividualCharactersTest)
// {
//   std::vector<string> input = {
//     "GACCA",
//     "ABCABCD",
//     "GAB"
//   };

//   vector<vector<double>> output;
//   TfIdfEncoding<CharExtract::TokenType> encoder(0, false);

//   // Passing a empty string to encode characters
//   encoder.Encode(input, output, CharExtract());
//   vector<vector<double>> expected = {
//     { 1.4054651081081644, 2, 2.8109302162163288, 0, 0 },
//     { 0, 2, 2.8109302162163288, 2.8109302162163288, 2.0986122886681100 },
//     { 1.4054651081081644, 1, 0, 1.4054651081081644, 0 }
//   };
//   for (size_t i = 0; i < expected.size(); i++)
//     for (size_t j = 0; j < expected[i].size(); j++)
//       BOOST_REQUIRE_CLOSE(expected[i][j], output[i][j], 1e-12);
// }

// /**
//  * Test TFIDF encoding for characters using lamda function, using binary
//  * weighting scheme for tf and smoothidf as true.
//  */
// BOOST_AUTO_TEST_CASE(BinarySmoothIdfEncodingIndividualCharactersTest)
// {
//   vector<string> input = {
//     "GACCA",
//     "ABCABCD",
//     "GAB"
//   };

//   arma::mat output;
//   TfIdfEncoding<CharExtract::TokenType> encoder(1, true);

//   // Passing a empty string to encode charactersrawcountsmoothidftrue
//   encoder.Encode(input, output, CharExtract());
//   arma::mat target = {
//     { 1.2876820724517808, 1, 1.2876820724517808, 0, 0 },
//     { 0, 1, 1.2876820724517808, 1.2876820724517808, 1.6931471805599454 },
//     { 1.2876820724517808, 1, 0, 1.2876820724517808, 0 }
//   };
//   CheckMatrices(output, target, 1e-12);
// }

// /**
//  * Test TFIDF encoding for characters using lamda function, using binary
//  * weighting scheme for tf and smoothidf as true.
//  */
// BOOST_AUTO_TEST_CASE(OnePassBnarySmoothIdfEncodingIndividualCharactersTest)
// {
//   std::vector<string> input = {
//     "GACCA",
//     "ABCABCD",
//     "GAB"
//   };

//   vector<vector<double>> output;
//   TfIdfEncoding<CharExtract::TokenType> encoder(1, true);

//   // Passing a empty string to encode characters
//   encoder.Encode(input, output, CharExtract());
//   vector<vector<double>> expected = {
//     { 1.2876820724517808, 1, 1.2876820724517808, 0, 0 },
//     { 0, 1, 1.2876820724517808, 1.2876820724517808, 1.6931471805599454 },
//     { 1.2876820724517808, 1, 0, 1.2876820724517808, 0 }
//   };
//   for (size_t i = 0; i < expected.size(); i++)
//     for (size_t j = 0; j < expected[i].size(); j++)
//       BOOST_REQUIRE_CLOSE(expected[i][j], output[i][j], 1e-12);
// }

// /**
//  * Test TFIDF encoding for characters using lamda function, using binary
//  * as weighting scheme and smoothidf as false.
//  */
// BOOST_AUTO_TEST_CASE(BinaryTfIdfEncodingIndividualCharactersTest)
// {
//   vector<string> input = {
//     "GACCA",
//     "ABCABCD",
//     "GAB"
//   };

//   arma::mat output;
//   TfIdfEncoding<CharExtract::TokenType> encoder(1, false);

//   // Passing a empty string to encode charactersrawcountsmoothidftrue
//   encoder.Encode(input, output, CharExtract());
//   arma::mat target = {
//     { 1.4054651081081644, 1, 1.4054651081081644, 0, 0 },
//     { 0, 1, 1.4054651081081644, 1.4054651081081644, 2.0986122886681100 },
//     { 1.4054651081081644, 1, 0, 1.4054651081081644, 0 }
//   };
//   CheckMatrices(output, target, 1e-12);
// }

// /**
//  * Test TFIDF encoding for characters using lamda function, using sublinear
//  * as weighting scheme and smoothidf as true.
//  */
// BOOST_AUTO_TEST_CASE(SublinearSmoothIdfEncodingIndividualCharactersTest)
// {
//   vector<string> input = {
//     "GACCA",
//     "ABCABCD",
//     "GAB"
//   };

//   arma::mat output;
//   TfIdfEncoding<CharExtract::TokenType> encoder(2, true);

//   // Passing a empty string to encode charactersrawcountsmoothidftrue
//   encoder.Encode(input, output, CharExtract());
//   arma::mat target = {
//     { 1.2876820724517808, 1.6931471805599454, 2.1802352704293200, 0, 0 },
//     { 0, 1.6931471805599454, 2.1802352704293200, 2.1802352704293200,
//       1.6931471805599454 },
//     { 1.2876820724517808, 1, 0, 1.2876820724517808, 0 }
//   };
//   CheckMatrices(output, target, 1e-12);
// }

// /**
//  * Test TFIDF encoding for characters using lamda function, using sublinear
//  * as weighting scheme and smoothidf as false.
//  */
// BOOST_AUTO_TEST_CASE(SublinearTfIdfEncodingIndividualCharactersTest)
// {
//   vector<string> input = {
//     "GACCA",
//     "ABCABCD",
//     "GAB"
//   };

//   arma::mat output;
//   TfIdfEncoding<CharExtract::TokenType> encoder(2, false);

//   // Passing a empty string to encode charactersrawcountsmoothidftrue
//   encoder.Encode(input, output, CharExtract());
//   arma::mat target = {
//     { 1.4054651081081644, 1.6931471805599454, 2.3796592851687173, 0, 0 },
//     { 0, 1.6931471805599454, 2.3796592851687173, 2.3796592851687173,
//       2.0986122886681100 },
//     { 1.4054651081081644, 1, 0, 1.4054651081081644, 0 }
//   };
//   CheckMatrices(output, target, 1e-12);
// }

// /**
//  * Test TFIDF encoding for characters using lamda function, using term
//  * Frequency as weighting scheme and smoothidf as true.
//  */
// BOOST_AUTO_TEST_CASE(TermFrequencySmoothIdfEncodingIndividualCharactersTest)
// {
//   vector<string> input = {
//     "GACCA",
//     "ABCABCD",
//     "GAB"
//   };

//   arma::mat output;
//   TfIdfEncoding<CharExtract::TokenType> encoder(3, true);

//   // Passing a empty string to encode charactersrawcountsmoothidftrue
//   encoder.Encode(input, output, CharExtract());
//   arma::mat target = {
//     { 0.2575364144903562, 0.4, 0.5150728289807124, 0, 0 },
//     { 0, 0.2857142857142857, 0.3679091635576516, 0.3679091635576516,
//       0.2418781686514208 },
//     { 0.4292273574839269, 0.3333333333333333, 0, 0.4292273574839269, 0 }
//   };
//   CheckMatrices(output, target, 1e-12);
// }

// /**
//  * Test TFIDF encoding for characters using lamda function, using Term
//  * Frequency as weighting scheme and smoothidf as false.
//  */
// BOOST_AUTO_TEST_CASE(TermFrequencyTfIdfEncodingIndividualCharactersTest)
// {
//   vector<string> input = {
//     "GACCA",
//     "ABCABCD",
//     "GAB"
//   };

//   arma::mat output;
//   TfIdfEncoding<CharExtract::TokenType> encoder(3, false);

//   // Passing a empty string to encode charactersrawcountsmoothidftrue
//   encoder.Encode(input, output, CharExtract());
//   arma::mat target = {
//     { 0.2810930216216329, 0.4, 0.5621860432432658, 0, 0 },
//     { 0, 0.2857142857142857, 0.4015614594594755, 0.4015614594594755,
//       0.2998017555240157 },
//     { 0.4684883693693881, 0.3333333333333333, 0, 0.4684883693693881, 0 }
//   };
//   CheckMatrices(output, target, 1e-12);
// }

/**
 * Serialization test for the TF-IDF encoding algorithm with
 * the CharExtract tokenizer.
 */
BOOST_AUTO_TEST_CASE(CharExtractTfIdfEncodingSerialization)
{
  using EncoderType = TfIdfEncoding<CharExtract::TokenType>;

  EncoderType encoder;
  CharExtract tokenizer;
  arma::mat output;
  vector<string> input = {
    "GACCA",
    "ABCABCD",
    "GAB"
  };
  encoder.Encode(input, output, tokenizer);

  EncoderType xmlEncoder, textEncoder, binaryEncoder;
  arma::mat xmlOutput, textOutput, binaryOutput;

  SerializeObjectAll(encoder, xmlEncoder, textEncoder, binaryEncoder);
  std::cout<<"HHHHHHHHHHHHHH\n";
  CheckDictionaries(encoder.Dictionary(), xmlEncoder.Dictionary());
  CheckDictionaries(encoder.Dictionary(), textEncoder.Dictionary());
  CheckDictionaries(encoder.Dictionary(), binaryEncoder.Dictionary());
  std::cout<<"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFSDDDDDDDDDDDDDDDD\n";
  xmlEncoder.Encode(input, xmlOutput, tokenizer);
  std::cout<<"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n";

  textEncoder.Encode(input, textOutput, tokenizer);
  binaryEncoder.Encode(input, binaryOutput, tokenizer);

  CheckMatrices(output, xmlOutput, textOutput, binaryOutput);
}

BOOST_AUTO_TEST_SUITE_END();


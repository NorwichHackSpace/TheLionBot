/*******************************************************************************
 The Lion: Norwich Hackspace's very own bot for Slack and Lion House automation
 Name        : jokes.cpp
 Authors     : Alan Percy Childs, James Tayloe
 Version     :

 For designing expressions you might want to try -> https://regexr.com/

*******************************************************************************/

#include "../slack.hpp"
#include <regex>
#include "../rest.hpp"
#include "../TheLionBot.hpp"

using namespace std;

response jokes;
std::string slack::joke_xmas( ) {
	string responses[] = {
			"What goes 'Oh, Oh, Oh'? Santa walking backwards",
			"Why are Christmas trees bad knitters? They keep losing their needles",
			"What do you call a line of men waiting for a haircut? A barberqueue",
			"Why was the turkey in the pop group? Because he was the only one with drumsticks!",
			"What do snowmen wear on their heads? Ice caps",
			"Why was the snowman looking through the carrots? He was picking his nose",
			"Two snowmen were standing in a field. One said , 'Can you smell carrots?'",
			"What did Adam say the day before Christmas? 'It's Christmas, Eve'",
			"What does Santa do with fat elves? He sends them to an Elf Farm",
			"What did Cinderella say when her photos didn’t arrive on time? One day my prints will come",
			"What’s a dog’s favourite carol? Bark, the herald angels sing",
			"What does Miley Cyrus have for her Christmas dinner? Twerky",
			"What do snowmen have for breakfast? Snowflakes",
			"What does Father Christmas do when his elves misbehave? He gives them the sack",
			"What do you give a dog for Christmas? A mobile bone",
			"Why did the pony have to gargle? Because it was a little horse",
			"What goes Ho Ho Whoosh, Ho Ho Whoosh, Ho Ho Whoosh? Father Christmas in a revolving door",
			"What is Santa’s favourite pizza? One that’s deep-pan, crisp and even",
			"What do Santa's little helpers learn at school? The elf-abet!",
			"What’s a horse’s favourite TV show? Neighbours",
			"What do you get when you cross a snowman with a vampire? Frostbite",
			"What do you call a train loaded with toffee? A chew chew train",
			"Why couldn’t the skeleton go to the Christmas party? He had no body to go with",
			"Why did no-one bid for Rudolph and Blitzen on eBay? Because they were two deer",
			"What happened to the man who stole an advent calendar? He got 25 days",
			"How do snowmen get around? By riding an ‘icicle",
			"How did Mary and Joseph know that Jesus was 7lb 6oz when he was born? They had a weigh in a manger",
			"Who hides in the bakery at Christmas? A mince spy",
			"What is the best Christmas present? A broken drum, you can't beat it!",
			"What do you call a woman who stands between two goal posts? Annette",
			"What has four legs but can’t walk? A table",
			"Why did Santa have to go to the hospital? Because of his poor elf",
			"What do frogs wear on their feet? Open toad sandles",
			"Why are pirates called pirates? Because they arrrrrrr!",
			"What do you call a blind reindeer? No-eye deer",
			"What’s round and bad tempered? A vicious circle"
			"A friend bought his wife a artificial leg for Christmas... Wasn't the main present.. Just a stocking filler. :socks: " //Submitted by Andy S
			"Where do snowmen keep their money? In a snow bank." //Submitted bt Ben N
	};
	int size = ((&responses)[1] - responses);
	string response = responses[jokes.sequence(size)];
	return response;
};

std::string slack::joke( ) {
	if ( isChristmas() ) { return slack::joke_xmas(); }
	//Nicolas Cage jokes requested by Dan R
	if (rand() % 2) { //50% odds
		//fetch
		std::string res = fetch::https(
					 "api.icndb.com",
					 "/jokes/random/?firstName=Nicolas&lastName=Cage&exclude=[explicit]"
			);
		//Do
		rapidjson::Document replyJSON;
		 replyJSON.Parse( res.c_str() );
		 string NicolasCage = replyJSON["value"]["joke"].GetString();
		srand(time(0));
		if ( !(NicolasCage == "") ) { return NicolasCage; }
	}

	string responses[] = {
			"Knock! Knock! Who's there? Dejav. Dejav who? Knock! Knock!  "
			, "A man walks into a bar ...ouch"
			, "What do you call a boomerang that does not come back? A stick"
			, "Knock! Knock! Who's there? Oink oink. Oink oink who? Make up your mind—are you a pig, or an owl?!"
			, "What’s the best thing about Switzerland?   I don’t know, but the flag is a big plus."
			, "I invented a new word!     Plagiarism!"
			, "Did you hear about the mathematician who’s afraid of negative numbers?    He’ll stop at nothing to avoid them."
			, "Why do we tell actors to “break a leg?”    Because every play has a cast."
			, "Yesterday I saw a guy spill all his Scrabble letters on the road. I asked him, “What’s the word on the street?”"
			, "Hear about the new restaurant called Karma?   There’s no menu: You get what you deserve."
			, "A woman in labour suddenly shouted, “Shouldn’t! Wouldn’t! Couldn’t! Didn’t! Can’t!” “Don’t worry,” said the doctor. “Those are just contractions.”"
			, "Did you hear about the claustrophobic astronaut? He just needed a little space."
			, "Why don’t scientists trust atoms? Because they make up everything."
			, "Where are average things manufactured?   The satisfactory."
			, "How do you drown a hipster?  Throw him in the mainstream."
			, "Why are pirates called pirates? Because they arrgh!"
			, "A man tells his doctor, “Doc, help me. I’m addicted to Twitter!”  The doctor replies, “Sorry, I don’t follow you…”"
			, "What does Charles Dickens keep in his spice rack?   The best of thymes, the worst of thymes."
			, "What’s the different between a cat and a comma?  A cat has claws at the end of paws; A comma is a pause at the end of a clause."
			, "What did the bald man exclaim when he received a comb for a present?  Thanks— I’ll never part with it!"
			, "What did the left eye say to the right eye?   Between you and me, something smells."
			, "What do you call a pony with a cough?  A little horse."
			, "What did one hat say to the other?   You wait here. I’ll go on a head."
			, "What do you call a magic dog?  A labracadabrador."
			, "What did the shark say when he ate the clownfish?    This tastes a little funny."
			, "I waited all night to see where the sun would rise… And then it dawned on me."
			, "What did the pirate say when he turned 80?   Aye matey."
			, "Why did the lion cross the road?  To catch the chicken." //Submitted by Nicolas and Marion
	};
	int size = ((&responses)[1] - responses);
	string response = responses[jokes.sequence(size)];
	return response;
};


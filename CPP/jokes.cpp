/*******************************************************************************
 The Lion: Norwich Hackspace's very own bot for Slack and Lion House automation
 Name        : jokes.cpp
 Authors     : Alan Percy Childs, James Tayloe
 Version     :

 For designing expressions you might want to try -> https://regexr.com/

*******************************************************************************/

#include "TheLionBot.hpp"
#include <regex>

using namespace std;

response jokes;
std::string xmas_joke( ) {
	string responses[] = {
			"What goes 'Oh, Oh, Oh'? Santa walking backwards",
			"Why are Christmas trees bad knitters? They keep losing their needles",
			"What do you call a line of men waiting for a haircut? A barberqueue",
			"Why was the turkey in the pop group? Because he was the only one with drumsticks!",
			"What do you call a boomerang that does not come back? A stick",
			"What do snowmen wear on their heads? Ice caps",
			"Why was the snowman looking through the carrots? He was picking his nose",
			"Two snowmen were standing in a field. One said , 'Can you smell carrots?'",
			"A man walks into a bar ...ouch",
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
			"What’s round and bad tempered? A vicious circle",
	};
	int size = ((&responses)[1] - responses);
	string response = responses[jokes.sequence(size)];
	return response;
};


Warlords and Scumbags
========
 This provides a client and server for the card game.  
Created for CSCI 367 Networking Class at WWU Fall 2013.

Warlords and Scumbags is a card game in which three to seven players race to get rid of all the cards in their hand in order to become the Warlord of the next round. The last player who still has cards in his or her hand is the Scumbag and must give the Warlord his or her highest card, in exchange for any card the Warlord selects, before play commences after the deal.

The cards suits do not matter, only the face value is used to determine a cards rank. In this version  the ordinality of the cards increses from 3 up to king, and then ace, and finally two. The twos are treated as trumps.

For the deal, all cards are dolled out to the players as evenly as possible. If it is not the first hand of a game--and so there is not a Warlord/Scumbag ranking status--then the person who holds the 3 of clubs begins by playing a set of one to four cards which contains the 3 of clubs. Valid sets contain cards which all share the same face value, for example three threes.

In order to be able to play cards on a players turn, he or she must have a set which beats the previous play. If he or she cannot, then the player must pass. One set of cards beats another if it has both an equal or greater quantity of cards and an equal or greater face value of cards. If the play matches the previous play--that is it has an equal quantity of cards of equal face value, then the next player is forced to pass.

Having passed a player is not allowed to play until the table is reset. This occurs when all active players have passed, or when someone plays a trump.

[TODO: finish gameplay explanation]


read man ./warlords.6

auto_cli.exp is an tcl script using expect and telnet to connect to a server and play poorly.

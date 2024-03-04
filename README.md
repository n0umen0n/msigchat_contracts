# msig.chat contracts overview

The contract is in production on WAX blockchain, deployed to following [chat account](https://waxblock.io/account/chat). 

Contract for testing is deployed to following [test.chat account](https://waxblock.io/account/test.chat). 

Contract contains following TABLES:

chats1 - stores all the info about the group chat (community name, description etc.).
messagetb2 - stores all the messages.
profile - stores information about the users who created profiles.
locproposal1 - stores information in regards to msig proposals.
pinmessage5 - stores pinned messages in the chat.


ACTIONS can be categorized into two groups. One's that require authentication from the user's account and one's that require our contracts authentication. Once the user successfully proves the ownership of hiw WAX account when connecting to the chat, websocket connection is established that enables our server to execute some of the ACTIONS on the user's behalf. Such system makes the UX very smooth, as user does not have to open a wallet (Anchor) to sign a trx. Custom permission is created for the [chat](https://waxblock.io/account/chat) account. Actions that need to be linked to that permission can be found [here](https://waxblock.io/account/chat#keys). 


Contract is compiled utilizing [DUNES](https://github.com/AntelopeIO/DUNES).


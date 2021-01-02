# ResearchProjectGP
## Finite state machines and behavior trees

### Goal
I will look into both finite state machines and behavior trees to find out their pro's and cons, then combine these two decision making structures to try and keep as many of these pro's and negate or reduce the cons.

### FSM
Finite state machines, FSM for short, use states and transitions for their logic. A state *does* something, while a transition *checks* something. Normally, every state has at least one transition. One transition would be used to either go away from this state, in case this particular one only needs to be called once at the start of the game, or go to this state, for example when the character dies. States can have mulitple transitions, both incoming and outgoing, and with that, a first problem pops up: FSMs can get really complex really quickly. If you keep in mind that every state *can* have two transitions (back and forth) for every other state, and AI rarely has only 2 or 3 states, the structure of an FSM becomes almost spiderweb-like - only less clean and even less easy to manage. This complexity can make debugging of transitions really, really difficult.

If you FSM begins getting complex, another problem pops up. Take this simple scenario: a mouse goes out to search cheese ( = starting state), if he found cheese, he returns to his hole, and if the cursor somes to close, he runs away.

![alt text](https://github.com/BlackSilverFox/ResearchProjectGP/blob/main/SimpleFSM_1.png)

THis will give no problems: when the cursor moves away, the mouse will automatically go back to finding cheese. Yet with this small FSM, there is one weird thing going on: once th mouse has cheese, he will not run away from the cursor anymore. If this cursor represents a cat, this mouse is, at the moment, quit suicidal, no? TO fix this, we can do this:

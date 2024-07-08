All the code inside the repository should follow this convention, and I encourage modders to do the same to maintain consistency.

## General Rules
- Descriptive names - avoid abbreviations, care about yourself in 2 days.
- Long functions - single, sequential large chunks of code are easier to understand than a spaghetti that goes through 15 different functions scattered around.
- Keep state together - concentrate all the state of your module in a single place, this helps to keep track of everything happening.
- 1 external header - put your mod's whole interface in a single header
## Style
The code follows this style:

```c
int category_object_function(){ // brackets like these
	// example: vehicles_car_accelerate() 
}

//   V  asterisk after
int  *pointer;

// avoids confusion sometimes
//    V pointer V
int   *a,  b,  *c;
//         /\ not pointer

typedef struct{
	int data;
}some_type_t;
// could be SomeType, this choice is aesthetic, but don't forget the _t!_

for(int i = 0; i < 10; i++){ // give it some space!
	// i, j, k  should always be just counters
}

```
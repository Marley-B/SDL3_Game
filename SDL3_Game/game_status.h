#pragma once
// manages the difrent game states the game can change through
//
//class GameStatus
//{
//public:
//    //State transitions
//    virtual bool enter() = 0;
//    //virtual bool exit() = 0;
//
//    //Main loop functions
//    virtual void handleEvent(SDL_Event& e) = 0;
//    virtual void update() = 0;
//    virtual void render() = 0;
//
//    //Make sure to call child destructors
//    virtual ~GameStatus() = default;
//};
//
//void setNextState(GameState* newState)
//{
//    //If the user doesn't want to exit
//    if (gNextState != ExitState::get())
//    {
//        //Set the next state
//        gNextState = newState;
//    }
//}
//
//bool changeState()
//{
//    //Flag successful state changes
//    bool success{ true };
//
//    //If the state needs to be changed
//    if (gNextState != nullptr)
//    {
//        success = gCurrentState->exit() && success;
//        success = gNextState->enter() && success;
//
//        //Change the current state ID
//        gCurrentState = gNextState;
//        gNextState = nullptr;
//    }
//
//    return success;
//}

// static IntroState* get();
//IntroState* IntroState::get()
//{
//    //Get static instance
//    return &sIntroState;
//}
#include "context.h"

ContextEditor::ContextEditor(){
	interface = Handle<TextInterfaceBase>(new CursesInterface());
	
	ModeBase* mode = new EditMode();

	int w,h;
	
	w = interface->GetWidth();
	h = interface->GetHeight();
	interface->WindowResized(w,h);

	modes.push_back(Handle<ModeBase>(mode));
	currentMode = 0;
}

void ContextEditor::Loop(){
	bool quit = false;
	KeyboardEvent* event;
	while (!quit){
		interface->RenderScreen(modes[currentMode]->GetTextScreen(interface->GetWidth(),interface->GetHeight()));

		if ((event = interface->GetKeyboardEvent())){
			if (event->key=='q'&&event->mod==KeyModifier::Ctrl)
				quit = true;

			modes[currentMode]->ProcessKeyboardEvent(event);

		}
	}
}

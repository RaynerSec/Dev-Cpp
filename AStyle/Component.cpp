#include "Component.h"

Component::Component(int left,int top,int width,int height) {

	// Misc
	type = ctBase;
	backcolor = float4(0.3f,0.3f,0.3f,1.0f);
	parent = NULL;
	OnShow = NULL;

	// Deze zijn relatief aan de parent
	this->left = 0;
	this->top = 0;
	this->absleft = 0;
	this->abstop = 0;
	Move(left,top);

	// Deze zijn absoluut
	this->width = width;
	this->height = height;

	// Initial value: not visible
	visible = false;
	plane = NULL;

	// But try to show anyway
	Show(true);
}
Component::~Component() {

	// Delete children...
	for(unsigned int i = 0; i < children.size(); i++) { // recursief...
		switch(children[i]->type) {
			case ctBase: {
				delete (Component*)children[i];
				break;
			}
			case ctButton: {
				delete (Button*)children[i];
				break;
			}
			case ctLabel: {
				delete (Label*)children[i];
				break;
			}
			case ctWindow: {
				delete (Window*)children[i];
				break;
			}
			case ctEdit: {
				delete (Edit*)children[i];
				break;
			}
			case ctBevel: {
				delete (Bevel*)children[i];
				break;
			}
			case ctDropdown: {
				delete (Dropdown*)children[i];
				break;
			}
		}
	}
	children.clear();

	// Then delete our model
	models->Delete(plane);

	// Remove children too
}
void Component::CreatePlane() {
	if(!renderer) {
		return; // create it OnShow then...
	}

	// Use screen space coordinates
	float2 screenspacesize = renderer->PixelsToProjection(float2(width,height));

	// Reuse existing object
	if(!plane) {
		plane = models->Add();
	}
	plane->Load2DQuad(-1.0f,-1.0f,screenspacesize.x,
	                  -screenspacesize.y); // apply transforms to move
	plane->SendToGPU();

	// Werk de children ook bij
	for(unsigned int i = 0; i < children.size(); i++) {
		children[i]->CreatePlane();
	}
}
void Component::Move(int dx,int dy) {

	// Verschuif mezelf
	left += dx;
	top += dy;
	absleft += dx;
	abstop += dy;

	if(renderer) {

		// Use screen space coordinates
		float2 screenspacecorner = renderer->PixelsToProjection(float2(absleft,abstop));

		// Move by applying translation
		matWorld.Translation(
		    float3(
		        screenspacecorner.x+1.0f,
		        screenspacecorner.y-1.0f,
		        0));
	}

	// Werk de children bij
	for(unsigned int i = 0; i < children.size(); i++) {
		children[i]->Move(dx,dy);
	}
}
void Component::SetPos(int x,int y) {
	Move(x - left,y - top);
}
void Component::Resize(int width,int height) {
	this->width = width;
	this->height = height;
	CreatePlane(); // recreate buffer
}
void Component::GetRect(RECT* result) {
	result->left = absleft;
	result->top = abstop;
	result->right = absleft + width;
	result->bottom = abstop + height;
}
void Component::AddChild(Component* child) {
	children.push_back(child);
	child->absleft = child->left + left;
	child->abstop = child->top + top;
	child->parent = this;
	child->Move(0,0);
}
void Component::OnResetDevice() {
	CreatePlane();
	Move(0,0);
}
bool Component::IsVisible() {
	return visible;
}
void Component::SetVisible(bool value) {
	if(visible != value) {
		visible = value;
		if(visible) {
			if(renderer && !plane) {
				OnResetDevice(); // create buffers for the first time
			}
			if(OnShow) {
				OnShow(NULL);
			}
		}
	}
}

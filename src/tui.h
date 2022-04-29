#pragma once

#include "core.h"

#include "textscreen.h"

class TUIElement {
protected:
	s32 x,y,w,h;
public:
	TUIElement() = default;
	TUIElement(s32 x,s32 y,s32 w,s32 h) : x(x),y(y),w(w),h(h){}

	virtual void Render(TextScreen&) const = 0;
};

class TUIBox : public TUIElement {
public:
	TUIBox(s32 x,s32 y,s32 w,s32 h) : TUIElement(x,y,w,h){}

	void Render(TextScreen&) const override;
};

class TUITextBox : public TUIBox {
	std::string titleText;
	std::string_view text;
	
public:
	TUITextBox(std::string_view sv,s32 x,s32 y,s32 w,s32 h) : TUIBox(x,y,w,h),titleText(),text(sv){}

	void SetTitle(const std::string& title);

	void Render(TextScreen&) const override;
};

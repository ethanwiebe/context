#include "tui.h"

s32 Resolve(s32 pos,s32 size){
	if (pos<0)
		return size+pos;

	return pos;
}

s32 ResolveSize(s32 innerSize,s32 size){
	if (innerSize<=0)
		return size+innerSize;

	return innerSize;
}


void TUIBox::Render(TextScreen& ts) const {
	s32 screenW = ts.GetWidth();
	s32 screenH = ts.GetHeight();
	
	s32 realX = Resolve(x,screenW);
	s32 realY = Resolve(y,screenH);
	s32 realW = ResolveSize(w,screenW);
	s32 realH = ResolveSize(h,screenH);

	s32 leftX = realX-1;
	s32 rightX = realX+realW;

	s32 topY = realY-1;
	s32 bottomY = realY+realH;

	//top line
	if (topY>=0){
		for (s32 xpos=leftX+1;xpos<rightX;++xpos){
			ts.SetAt(xpos,topY,TextCell(113,lineDrawingStyle));
		}

		ts.SetAt(leftX,topY,TextCell(108,lineDrawingStyle));
		ts.SetAt(rightX,topY,TextCell(107,lineDrawingStyle));
	}

	if (bottomY<screenH){
		for (s32 xpos=leftX+1;xpos<rightX;++xpos){
			ts.SetAt(xpos,bottomY,TextCell(113,lineDrawingStyle));
		}

		ts.SetAt(leftX,bottomY,TextCell(109,lineDrawingStyle));
		ts.SetAt(rightX,bottomY,TextCell(106,lineDrawingStyle));
	}

	if (leftX>=0){
		for (s32 ypos=topY+1;ypos<bottomY;++ypos){
			ts.SetAt(leftX,ypos,TextCell(120,lineDrawingStyle));
		}
	}

	if (rightX<screenW){
		for (s32 ypos=topY+1;ypos<bottomY;++ypos){
			ts.SetAt(rightX,ypos,TextCell(120,lineDrawingStyle));
		}
	}
}

void TUITextBox::SetTitle(const std::string& title){
	titleText = title;
}

void TUITextBox::Render(TextScreen& ts) const {
	TUIBox::Render(ts);

	s32 realW = ResolveSize(w,ts.GetWidth());
	s32 realH = ResolveSize(h,ts.GetHeight());
	s32 realX = Resolve(x,ts.GetWidth());
	s32 realY = Resolve(y,ts.GetHeight());

	if (!titleText.empty()){
		TextStyle titleStyle = defaultStyle;
		titleStyle.flags |= StyleFlag::Bold;
		s32 xpos = realX;
		s32 ypos = realY-1;
		for (const auto& c : titleText){
			ts.SetAt(xpos,ypos,TextCell(c,titleStyle));
			++xpos;
		}
	}

	s32 xpos = 0;
	s32 ypos = 0;

	for (const auto& c : text){
		if (c=='\n'||c=='\r'){ //newline handling
			while (xpos<realW){
				ts.SetAt(xpos+realX,ypos+realY,TextCell(' ',defaultStyle));
				++xpos;
			}
			xpos = 0;
			++ypos;
			if (ypos>realH-1) break;
			continue;
		}

		ts.SetAt(xpos+realX,ypos+realY,TextCell(c,defaultStyle));

		++xpos;
		if (xpos>realW-1){ //line wrap
			xpos = 0;
			++ypos;
			if (ypos>realH-1){
				break;
			}
		}
	}

	while (ypos<realH){
		while (xpos<realW){
			ts.SetAt(xpos+realX,ypos+realY,TextCell(' ',defaultStyle));
			++xpos;
		}
		xpos = 0;
		++ypos;
	}
}

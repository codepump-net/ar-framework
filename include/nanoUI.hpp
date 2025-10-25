//
//  nanoUI.hpp
//  GLUK
//
//  Created by Hyun Joon Shin on 26/04/2018.
//  Copyright © 2018 Hyun Joon Shin. All rights reserved.
//

#ifndef nanoUI_hpp
#define nanoUI_hpp

#include <string>
#include <vector>

#ifdef NANOUI_IMPLEMENTATION
#define NANOVG_IMPLEMENTATION
#define NANOVG_GL3_IMPLEMENTATION
#endif

#include "nanovg.h"
#include "nanovg_gl.h"
#include "math.h"

enum NANO_UI_MOD {
	MOD_NONE  = 0,
	MOD_SHIFT_ = 1,
	MOD_CTRL  = 2,
	MOD_ALT_   = 4,
	MOD_SUPER = 8,
	MOD_LBTN  = 16,
	MOD_RBTN  = 32,
	MOD_BTN_MASK = 0xF0,
};


extern void nvgRegisterDefaultFonts( NVGcontext* vg, const std::string& path="" );
struct nanoRect {
	float _x, _y, _w, _h;
	nanoRect( float x, float y, float w, float h ): _x(x),_y(y),_w(w),_h(h){}
	nanoRect( float w, float h ): _x(0),_y(0),_w(w),_h(h){}
	nanoRect(): _x(0),_y(0),_w(0),_h(0){}
	nanoRect resizeLeft ( float w ) { return nanoRect( _x, _y, w, _h ); }
	nanoRect resizeRight( float w ) { return nanoRect( _x+_w-w, _y, w, _h ); }
	nanoRect offsetLeft ( float d ) { return nanoRect( _x, _y, _w-d, _h ); }
	nanoRect offsetRight( float d ) { return nanoRect( _x+d, _y, _w-d, _h ); }
	nanoRect resizeUp   ( float h ) { return nanoRect( _x, _y, _w, h ); }
	nanoRect resizeDown ( float h ) { return nanoRect( _x, _y+_h-h, _w, h ); }
	nanoRect resizeHorz ( float w ) { return nanoRect( _x+(_w-w)/2, _y, w, _h ); }
	nanoRect resizeVert ( float h ) { return nanoRect( _x, _y+(_h-h)/2, _w, h ); }
	bool inside( float x, float y ) { return x>=_x && y>=_y && x<_x+_w && y<_y+_h; }
};

struct nanoWidget {
	static float globalLabelPt;
	static std::string globalLableFont;
	static NVGcolor globalLabelColor;
	static NVGcolor globalLabelColorDisable;
	static NVGcolor globalGroupColor;

	nanoRect _rect;
	float margin =0;
	float padding=0;
	bool _enabled = true;
	bool _wAutosize = false;
	bool _hAutosize = false;
	std::string _title;
	int _titleAlign = NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE;
	nanoWidget( float x, float y, float w, float h, const std::string& title )
	:_rect(x,y,w,h), _title(title), _enabled(true),
	_wAutosize(_rect._w<=0), _hAutosize(_rect._h<=0){}

	virtual void render(NVGcontext* vg);
	virtual void updateSize(NVGcontext* vg);

	virtual bool press(float x, float y, int button, NANO_UI_MOD mods);
	virtual bool release(float x, float y, int button, NANO_UI_MOD mods);
	virtual bool move(float x, float y, NANO_UI_MOD mods );
	virtual nanoRect contentRect() const;
	virtual nanoRect backRect() const;

	virtual NVGcolor labelColor() const { return _enabled?globalLabelColor:globalLabelColorDisable; }
	virtual float labelPt() const { return globalLabelPt; }
	virtual const char* labelFont() const { return globalLableFont.c_str(); }
	virtual nanoRect  labelSize(NVGcontext* vg) const;
	virtual void drawLabel(NVGcontext* vg, const nanoRect& r ) const;
	virtual void drawDownBox(NVGcontext* vg, const nanoRect& r, bool checked=false) const;

	static NANO_UI_MOD _ui_mods;
#ifdef _glfw3_h_
	static bool nanoUIButton( GLFWwindow* win, int button, int action, int mods, nanoWidget& group ) {
		double mx, my;
		NANO_UI_MOD tempMods = MOD_NONE;
		if( button == GLFW_MOUSE_BUTTON_1 ) tempMods = MOD_LBTN;
		else if( button == GLFW_MOUSE_BUTTON_2 ) tempMods = MOD_RBTN;
		glfwGetCursorPos( win, &mx, &my );
		if( action == GLFW_PRESS ) {
			_ui_mods = (NANO_UI_MOD)(mods|((_ui_mods&MOD_BTN_MASK)|tempMods));
			return group.press( (float)mx, (float)my, button, _ui_mods );
		}
		else if( action == GLFW_RELEASE ) {
			_ui_mods = (NANO_UI_MOD)(mods|((_ui_mods&MOD_BTN_MASK)&(~tempMods)));
			return group.release( (float)mx, (float)my, button, (NANO_UI_MOD)mods );
		}
		return 0;
	}
	static bool nanoUIMove( GLFWwindow* win, double mx, double my, nanoWidget& group ) {
		return group.move( (float)mx, (float)my, _ui_mods);
	}
	static void nanoUIRender( NVGcontext* vg, int width, int height, nanoWidget& group ) {
		nvgBeginFrame(vg, width, height, 1);
		group.render(vg);
		nvgEndFrame(vg);
	}
	static NVGcontext* nanoUIInit(const std::string& path ) {
//		NVGcontext* vg = nvgCreateGL2(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
		NVGcontext* vg = nvgCreateGL3(0);
		nvgRegisterDefaultFonts( vg, path );
		return vg;
	}
#endif
};

struct nanoGroup: nanoWidget {
	std::vector<nanoWidget*> _widgets;
	bool _wScaleChild = true;
	bool _hPackChild = true;
	nanoGroup(float x=0, float y=0, float w=0, float h=0, const std::string& title="")
	:nanoWidget(x,y,w,h,title){ margin=5; padding=labelPt()*0.5f; }

	void add(nanoWidget* widget);
	virtual void render(NVGcontext* ctx);
	virtual void updateSize(NVGcontext* vg);

	virtual bool press(float x, float y, int button, NANO_UI_MOD mods);
	virtual bool release(float x, float y, int button, NANO_UI_MOD mods);
	virtual bool move(float x, float y, NANO_UI_MOD mods );
};


struct nanoCheck: nanoWidget {
	bool* _checked = nullptr;
	nanoCheck( float x, float y, float w, const std::string& title, bool& checked )
	:nanoWidget(x, y, w, 0, title ), _checked(&checked){}

	virtual void render(NVGcontext* vg);
	virtual void updateSize(NVGcontext* vg);
	virtual bool press(float x, float y, int button, NANO_UI_MOD mods);
};

struct nanoCheckR: nanoCheck {
	nanoCheckR( float x, float y, float w, const std::string& title, bool& checked )
	:nanoCheck(x, y, w, title, checked ) {}
	virtual void render(NVGcontext* vg);
};

struct nanoSliderBase: nanoWidget {
	int _dragging = 0;
	int  _sliderSize = 100;
	float _sh2 = 2;
	bool _stoppable = false;
	bool _stopped = false;
	virtual float getPos() const = 0;
	virtual float setPos( float p ) = 0;
	nanoSliderBase( float x, float y, float w, const std::string& title, bool stoppable=false )
	:nanoWidget(x, y, w, 0, title ), _stoppable(stoppable) {}
	virtual float knobRadius() const;
	virtual nanoRect sliderRect() const;
	virtual float knobX() const;
	virtual float knobY() const;
	
	virtual void render(NVGcontext* vg);
	virtual void updateSize(NVGcontext* vg);
	virtual bool press(float x, float y, int button, NANO_UI_MOD mods);
	virtual bool move(float x, float y, NANO_UI_MOD mods );
	virtual bool release(float x, float y, int button, NANO_UI_MOD mods);
};


template<typename T> struct nanoSlider: nanoSliderBase {
	T* _value = nullptr;
	T _stopValue;
	float _stepStop = 0.f;
	float _minVal = 0, _maxVal = 1;
	nanoSlider<T>( float x, float y, float w, const std::string& title,
				T minVal, T maxVal, T& value, bool stoppable=false )
	:nanoSliderBase(x, y, w, title, stoppable ),
	_value(&value), _minVal(float(minVal)), _maxVal(float(maxVal)), _stopValue(value) {}
	virtual float getPos() const {
		if( _value ) return (*_value-_minVal)/(_maxVal-_minVal);
		else return _minVal;
	}
	virtual float setPos( float p ) {
		if( _stoppable && _stopped ) return getPos();
		if( p<0 ) p=0;
		if( p>1 ) p=1;
		T v = p*(_maxVal-_minVal)+_minVal;
		if( _stoppable && _value && ( (v<_stopValue && *_value>_stopValue ) || (v>_stopValue && *_value<_stopValue ) )) {
			_stopped = true;
			v = _stopValue;
		}
		if( _value ) *_value = v;
		return getPos();
	}
};

typedef nanoSlider<float> nanoSliderF;
typedef nanoSlider<int> nanoSliderI;

template<> inline float nanoSlider<int>::setPos( float p ) {
	if( _stoppable && _stopped ) return getPos();
	if( p<0 ) p=0;
	if( p>1 ) p=1;
	int v = (int)round( p*(_maxVal-_minVal)+_minVal );
	if( _stoppable && _value && ( (v<_stopValue && *_value>_stopValue ) || (v>_stopValue && *_value<_stopValue ) )) {
		_stopped = true;
		v = _stopValue;
	}
	if( _value ) *_value = v;
	return getPos();
}

template<> inline float nanoSlider<float>::setPos( float p ) {
	if( _stoppable && _stopped ) return getPos();
	
	if( p<0 ) p=0;
	if( p>1 ) p=1;
	float v;
	if( _stepStop>0.0 )	{
		v= ((int)round( p*(_maxVal-_minVal)/_stepStop ))*_stepStop+_minVal;
	}
	else {
		v= p*(_maxVal-_minVal)+_minVal;
		if( _stoppable && _value && ( (v<_stopValue && *_value>_stopValue ) || (v>_stopValue && *_value<_stopValue ) )) {
			_stopped = true;
			v = _stopValue;
		}
	}
	if( _value ) *_value = v;
	return getPos();
}


#ifdef NANOUI_IMPLEMENTATION

float nanoWidget::globalLabelPt = 18;
std::string nanoWidget::globalLableFont = "sans";
//NVGcolor nanoWidget::globalLabelColor = nvgRGBA(255,255,255,160);
NVGcolor nanoWidget::globalLabelColor = nvgRGBA(255,255,255,255);
NVGcolor nanoWidget::globalLabelColorDisable = nvgRGBA(255,255,255,40);
//NVGcolor nanoWidget::globalGroupColor = nvgRGBA(2,2,3,192);
NVGcolor nanoWidget::globalGroupColor = nvgRGBA(28,30,34,192);
//NVGcolor nanoWidget::globalGroupColor = nvgRGBA(8,10,98,102);

#define CHECK_BOX_INNER_ALPHA 132
#define CHECK_BOX_OUTTER_ALPHA 192

NANO_UI_MOD nanoWidget::_ui_mods = (NANO_UI_MOD)0;

//!@ Utility function to register default fonts for NVGcontext
void nvgRegisterDefaultFonts( NVGcontext* vg, const std::string& path ) {
	std::string ppp = path+((path.length()>0)?"/":"");
	int fontNormal = nvgCreateFont(vg, "sans", (ppp+"Roboto-Regular.ttf").c_str());
	int fontBold =   nvgCreateFont(vg, "sans-bold", (ppp+"Roboto-Bold.ttf").c_str());
	int fontEmoji =  nvgCreateFont(vg, "emoji", (ppp+"NotoEmoji-Regular.ttf").c_str());
	nvgCreateFont(vg, "icons", (ppp+"entypo.ttf").c_str());
	
	nvgAddFallbackFontId(vg, fontNormal, fontEmoji);
	nvgAddFallbackFontId(vg, fontBold, fontEmoji);
}

inline void nvgShadowedText( NVGcontext* vg, float x, float y,
							const char* str,
							const NVGcolor& textColor,
							const NVGcolor& shadowColor=nvgRGBA(0,0,0,160) ) {
	const std::vector<float> dx = { 0, 0, 0,-1, 1 };
	const std::vector<float> dy = { 0,-1, 1, 0, 0 };
	nvgFontBlur(vg, 1);
	nvgFillColor(vg, shadowColor);
	for( int i=0; i<(int)dx.size(); i++ )
	nvgText(vg, x+dx[i],y+dy[i],str,NULL);
	nvgFontBlur(vg, 0);
	nvgFillColor(vg, textColor);
	nvgText(vg, x  ,y  ,str,NULL);
}

#define ICON_CHECK 0x2713

inline static char* cpToUTF8(int cp, char* str)
{
	int n = 0;
	if (cp < 0x80) n = 1;
	else if (cp < 0x800) n = 2;
	else if (cp < 0x10000) n = 3;
	else if (cp < 0x200000) n = 4;
	else if (cp < 0x4000000) n = 5;
	else if (cp <= 0x7fffffff) n = 6;
	str[n] = '\0';
	switch (n) {
		case 6: str[5] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x4000000;
		case 5: str[4] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x200000;
		case 4: str[3] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x10000;
		case 3: str[2] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x800;
		case 2: str[1] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0xc0;
		case 1: str[0] = cp;
	}
	return str;
}

nanoRect nanoWidget::labelSize(NVGcontext* vg ) const {
	nanoRect ret;
	nvgFontSize(vg, labelPt());
	nvgFontFace(vg, labelFont());
	nvgTextAlign(vg, NVG_ALIGN_TOP|NVG_ALIGN_LEFT);
	nvgTextBounds(vg, 0, 0, _title.c_str(), NULL, (float*)&ret);
	return ret;
}
nanoRect nanoWidget::contentRect() const {
	return nanoRect( _rect._x+margin+padding, _rect._y+margin+padding,
					_rect._w-margin*2-padding*2, _rect._h-margin*2-padding*2 );
}
nanoRect nanoWidget::backRect() const {
	return nanoRect( _rect._x+margin, _rect._y+margin,
					_rect._w-margin*2, _rect._h-margin*2 );
}

void nanoWidget::drawLabel(NVGcontext* vg, const nanoRect& r ) const {
	nvgSave(vg);
	nvgScissor(vg, r._x, r._y, r._w, r._h);
	nvgFontSize(vg, labelPt());
	nvgFontFace(vg, labelFont());
	nvgTextAlign(vg,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
	nvgShadowedText(vg, r._x,r._y+r._h/2,_title.c_str(), labelColor());
	nvgRestore(vg);
}

void nanoWidget::drawDownBox(NVGcontext* vg, const nanoRect& r, bool checked ) const {
	char icon[8];
	NVGpaint bg = nvgBoxGradient(vg, r._x+1,r._y+1,r._w,r._h, 3,3, nvgRGBA(0,0,0,CHECK_BOX_INNER_ALPHA), nvgRGBA(0,0,0,CHECK_BOX_OUTTER_ALPHA));
	nvgBeginPath(vg);
	nvgRoundedRect(vg, r._x,r._y, r._w,r._h, 3);
	nvgFillPaint(vg, bg);
	nvgFill(vg);
	if( checked ) {
		nvgFontSize(vg, labelPt()*2-2);
		nvgFontFace(vg, "icons");
		nvgTextAlign(vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
		nvgShadowedText(vg, r._x+r._w/2, r._y+r._h/2, cpToUTF8(ICON_CHECK,icon), labelColor());
	}
}

void nanoWidget::updateSize(NVGcontext* vg) {
	if( _wAutosize || _rect._w<=0 )
		_rect._w = labelSize(vg)._w+padding*2+margin*2;
	if( _hAutosize || _rect._h<=0 )
		_rect._h = labelSize(vg)._h+padding*2+margin*2;
}

void nanoWidget::render(NVGcontext* vg) {
	updateSize(vg);
	drawLabel(vg,contentRect());
	/*	float x = _rect._x+margin, y = _rect._y+margin;
	 float w = _rect._w-margin*2, h = _rect._h-margin*2;
	 nvgFontSize(vg, labelSize());
	 nvgFontFace(vg, labelFont());
	 nvgSave(vg);
	 nvgScissor(vg, x, y, w, h);
	 float xx = x, yy=y+h*.5f;
	 if( _titleAlign & NVG_ALIGN_CENTER ) xx = x+w/2;
	 if( _titleAlign & NVG_ALIGN_RIGHT  ) xx = x+w-2;
	 if( _titleAlign & NVG_ALIGN_MIDDLE ) yy = y+h/2;
	 if( _titleAlign & NVG_ALIGN_BOTTOM ) yy = y+h-2;
	 nvgTextAlign(vg,_titleAlign);
	 nvgShadowedText(vg, xx+1,yy,_title.c_str(), labelColor());
	 nvgRestore(vg);*/
}

bool nanoWidget::press(float x, float y, int button, NANO_UI_MOD mods){ return false; }
bool nanoWidget::release(float x, float y, int button, NANO_UI_MOD mods){ return false; }
bool nanoWidget::move(float x, float y, NANO_UI_MOD mods){ return false; }

void nanoGroup::updateSize(NVGcontext* vg) {
	for( auto& widget: _widgets ) if( widget )
		widget->updateSize(vg);
	if( _wAutosize || _rect._w<=0) for( auto& widget: _widgets ) if( widget ) {
		if( _rect._w < widget->_rect._w+widget->_rect._x+margin*2+padding*2 )
			_rect._w = widget->_rect._w+widget->_rect._x+margin*2+padding*2;
	}
	if( _wScaleChild ) {
		for( auto& widget: _widgets ) if( widget ) {
			widget->_rect._w = _rect._w-padding*2-margin*2;
			widget->_wAutosize = false;
		}
	}
	if( _hPackChild ) {
		float y = 0;
		for( auto& widget: _widgets ) if( widget ) {
			widget->_rect._y = y;
			y+=widget->_rect._h;
		}
	}
	if( _hAutosize || _rect._h<=0 ) for( auto& widget: _widgets ) if( widget ) {
		if( _rect._h < widget->_rect._h+widget->_rect._y+margin*2+padding*2 )
			_rect._h = widget->_rect._h+widget->_rect._y+margin*2+padding*2;
	}
}

void nanoGroup::render(NVGcontext* vg) {
	updateSize(vg);
	nanoRect backR = backRect();
	nanoRect contR = contentRect();
	if( backR._w>0 && backR._h>0 ) {
		float cornerR = 3.0f;
		NVGpaint shadowPaint;
		nvgBeginPath(vg);
		nvgRoundedRect(vg, backR._x, backR._y, backR._w, backR._h, cornerR);
		nvgFillColor(vg, globalGroupColor);
		nvgFill(vg);
		
		shadowPaint = nvgBoxGradient(vg, backR._x, backR._y+2, backR._w, backR._h,
									 cornerR*2, 10, nvgRGBA(0,0,0,128), nvgRGBA(0,0,0,0));
		nvgBeginPath(vg);
		nvgRect(vg, backR._x-10, backR._y-10, backR._w+20, backR._h+30);
		nvgRoundedRect(vg, backR._x, backR._y, backR._w, backR._h, cornerR);
		nvgPathWinding(vg, NVG_HOLE);
		nvgFillPaint(vg, shadowPaint);
		nvgFill(vg);
	}
	nvgSave(vg);
	nvgTranslate(vg, contR._x, contR._y);
	if( contR._w>0 && contR._h>0 ) nvgScissor(vg, 0, 0, contR._w, contR._h);
	for( auto& widget: _widgets ) if( widget ) widget->render(vg);
	nvgRestore(vg);
}

bool nanoGroup::press(float mx, float my, int button, NANO_UI_MOD mods) {
	nanoRect contR = contentRect();
	for( auto& widget: _widgets ) if( widget )
		if( widget->press(mx-contR._x,my-contR._y,button,mods) ) return true;
	return false;
}

bool nanoGroup::release(float mx, float my, int button, NANO_UI_MOD mods){
	nanoRect contR = contentRect();
	for( auto& widget: _widgets ) if( widget )
		if( widget->release(mx-contR._x,my-contR._y,button,mods) ) return true;
	return false;
}

bool nanoGroup::move(float mx, float my, NANO_UI_MOD mods){
	nanoRect contR = contentRect();
	for( auto& widget: _widgets ) if( widget )
		if( widget->move(mx-contR._x,my-contR._y,mods) ) return true;
	return false;
}

void nanoGroup::add(nanoWidget* widget) {
	_widgets.push_back(widget);
}

void nanoCheck::updateSize(NVGcontext* vg) {
	if( _wAutosize || _rect._w<=0 )
		_rect._w = labelSize(vg)._w+padding*2+margin*2+labelPt()*1.5f;
	if( _hAutosize || _rect._h<=0 )
		_rect._h = labelPt()*1.5f;
}

void nanoCheck::render(NVGcontext* vg) {
	updateSize(vg);
	drawLabel(vg, contentRect().offsetRight( labelPt()*1.5f ) );
	bool checked = false;
	if( _checked && *_checked ) checked = true;
	drawDownBox(vg, contentRect().resizeVert(labelPt()).resizeLeft(labelPt() ), checked);
}

bool nanoCheck::press(float x, float y, int button, NANO_UI_MOD mods) {
	if( contentRect().inside(x, y) ) {
		if( _checked ) *_checked = !*_checked;
		return true;
	}
	return false;
}

void nanoCheckR::render(NVGcontext* vg) {
	updateSize(vg);
	drawLabel(vg, contentRect().offsetLeft( labelPt()*1.5f ));
	bool checked = false;
	if( _checked && *_checked ) checked = true;
	drawDownBox(vg, contentRect().resizeRight( labelPt() ).resizeVert( labelPt() ), checked );
}


float nanoSliderBase::knobRadius() const {
	return contentRect()._h*0.3f;
}

nanoRect nanoSliderBase::sliderRect() const {
	nanoRect cr = contentRect();
	float kr = knobRadius();
	return nanoRect( cr._x+cr._w-_sliderSize+kr, cr._y+cr._h/2-_sh2,
					_sliderSize-kr*2, _sh2*2 );
}

float nanoSliderBase::knobX() const {
	nanoRect sr = sliderRect();
	float pos = getPos();
	return sr._x+pos*(sr._w-1);
}

float nanoSliderBase::knobY() const {
	nanoRect cr = contentRect();
	return cr._y+cr._h/2;
}

void nanoSliderBase::render(NVGcontext* vg) {
	updateSize(vg);
	float kr = knobRadius();
	nanoRect sliderR = sliderRect();
	float kx = knobX(), ky = knobY();
	
	drawLabel(vg, contentRect().offsetLeft(_sliderSize+labelPt()*.5f) );
	drawDownBox(vg, sliderR.resizeHorz(sliderR._w+_sh2*2 ) );
	
	// Knob Shadow
	NVGpaint bg = nvgRadialGradient(vg, kx, ky+1, kr-3,kr+3, nvgRGBA(0,0,0,64), nvgRGBA(0,0,0,0));
	nvgBeginPath(vg);
	nvgRect(vg, kx-kr-5, ky-kr-5,kr*2+5+5,kr*2+5+5+3);
	nvgCircle(vg, kx,ky, kr);
	nvgPathWinding(vg, NVG_HOLE);
	nvgFillPaint(vg, bg);
	nvgFill(vg);
	
	// Knob
	NVGpaint knob = nvgLinearGradient(vg, kx,ky-kr,kx,ky+kr, nvgRGBA(255,255,255,116), nvgRGBA(64,64,64,116));
	nvgBeginPath(vg);
	nvgCircle(vg, kx, ky, kr-1);
	nvgFillColor(vg, nvgRGBA(40,43,48,255));
	nvgFill(vg);
	nvgFillPaint(vg, knob);
	nvgFill(vg);
	
	nvgBeginPath(vg);
	nvgCircle(vg, kx, ky, kr-0.5f);
	nvgStrokeColor(vg, nvgRGBA(0,0,0,92));
	nvgStroke(vg);
}
void nanoSliderBase::updateSize(NVGcontext* vg) {
	if( _wAutosize || _rect._w<=0 )
		_rect._w = labelSize(vg)._w+padding*2+margin*2+_sliderSize+labelPt()*0.5f;
	if( _hAutosize || _rect._h<=0 )
		_rect._h = labelPt()*1.5f;
}

bool nanoSliderBase::press(float x, float y, int button, NANO_UI_MOD mods) {
	float kx = knobX(), ky = knobY(), kr = knobRadius();
	nanoRect sliderR = sliderRect();
	if( (x-kx)*(x-kx)+(y-ky)*(y-ky)< kr*kr ) {
		_dragging = true;
		return true;
	}
	else if( sliderR.resizeVert(_sh2*4).inside( x, y ) ) {
		setPos( (x-sliderR._x)/(sliderR._w-1) );
		return true;
	}
	_stopped = false;
	return false;
}

bool nanoSliderBase::move(float x, float y, NANO_UI_MOD mods ){
	nanoRect sliderR = sliderRect();
	if( _dragging ) {
		setPos( (x-sliderR._x)/(sliderR._w-1) );
		return true;
	}
	return false;
}

bool nanoSliderBase::release(float x, float y, int button, NANO_UI_MOD mods) {
	_dragging = false;
	_stopped = false;
	return false;
}


#endif



#endif /* nanoUI_hpp */

#include <Core/config.h>

#ifdef PLATFORM_COCOA

#include <Carbon/Carbon.h> 

#endif

#include <CtrlLib/CtrlLib.h>

#ifdef GUI_COCOA

#include <CtrlCore/CocoMM.h>
#include "ChCocoMM.h"

void Coco_PaintCh(void *cgcontext, int type, int value, int state)
{
	auto cg = (CGContextRef) cgcontext	;
	if(Upp::IsUHDMode())
		CGContextScaleCTM(cg, 2, 2);
	CGRect cr = CGRectMake(0, 0, 140, 140);
	if(type == COCO_NSCOLOR) {
		CGContextSaveGState(cg);
		CGContextSetFillColorWithColor(cg, Upp::decode(value,
			COCO_PAPER, [NSColor textBackgroundColor].CGColor,
			COCO_SELECTEDTEXT, [NSColor selectedTextColor].CGColor,
			COCO_SELECTEDPAPER, [NSColor selectedTextBackgroundColor].CGColor,
			COCO_DISABLED, [NSColor disabledControlTextColor].CGColor,
			COCO_WINDOW, [NSColor windowBackgroundColor].CGColor,
			COCO_SELECTEDMENUTEXT, [NSColor selectedMenuItemTextColor].CGColor,
			[NSColor textColor].CGColor
		));
		CGContextFillRect(cg, cr);
		CGContextRestoreGState(cg);
	}
	else
	if(type == COCO_NSIMAGE) {
		NSImage *img = [NSImage imageNamed:(value ? NSImageNameInfo : NSImageNameCaution)];
	    NSGraphicsContext *gc = [NSGraphicsContext graphicsContextWithCGContext:cg flipped:YES];
	    NSGraphicsContext* cgc = [NSGraphicsContext currentContext];
	    [NSGraphicsContext setCurrentContext:gc];
	    [img drawInRect:NSMakeRect(0, 0, 48, 48)];
		[NSGraphicsContext setCurrentContext:cgc];
	}
	else {
	    CGContextSaveGState(cg);
	    [NSGraphicsContext saveGraphicsState];
	    [NSGraphicsContext setCurrentContext:
			[NSGraphicsContext graphicsContextWithCGContext:cg flipped:YES]];
	
	    const CGRect dirtyRect = CGRectMake(20, 20, 100, 100);
	
		if(Upp::findarg(type, COCO_SCROLLTHUMB, COCO_SCROLLTRACK) >= 0) {
			NSScroller *scroller = [[NSScroller alloc] initWithFrame:NSMakeRect(0, 0, 200, 20)];
		    scroller.floatValue = 0;
		    scroller.knobProportion = 1;
			scroller.knobStyle = NSScrollerKnobStyleDefault;
			scroller.scrollerStyle = NSScrollerStyleLegacy;
			scroller.frame = cr;
			if(type == COCO_SCROLLTHUMB)
				[scroller drawKnob];
			else
				[scroller drawKnobSlotInRect:dirtyRect highlight:YES];
			[scroller release];
		}
		else
		if(type == COCO_TEXTFIELD) {
			NSTextField *tf = [[NSTextField alloc] init];
			tf.enabled = YES;
			tf.editable = YES;
			tf.bezeled = YES;
			tf.frame = CGRectMake(0, 0, 140, 40);
			[tf drawRect:dirtyRect];
			[tf release];
		}
		else {
			NSButton *bc = type == COCO_POPUPBUTTON ? [[NSPopUpButton alloc] init] : [[NSButton alloc] init];
			bc.allowsMixedState = type == COCO_CHECKBOX;
			bc.title = @"";
			bc.controlSize = type == COCO_RADIOBUTTON ? NSControlSizeSmall : NSControlSizeRegular;
			bc.frame = cr;
			bc.buttonType = Upp::decode(type, COCO_CHECKBOX, NSButtonTypeSwitch, COCO_RADIOBUTTON, NSButtonTypeRadio, NSButtonTypePushOnPushOff);
			bc.bezelStyle = type == COCO_BUTTON ? NSBezelStyleRounded : NSBezelStyleRegularSquare;
			bc.state = Upp::decode(value, 0, NSControlStateValueOff, 1, NSControlStateValueOn, NSControlStateValueMixed);
	        [bc highlight: state == Upp::CTRL_PRESSED];
			bc.enabled = state != Upp::CTRL_DISABLED;
			[bc drawRect:dirtyRect];
			[bc release];
		}
	
	    [NSGraphicsContext restoreGraphicsState];
	    CGContextRestoreGState(cg);
	}
}

#endif

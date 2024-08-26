// compile with:
// gcc example.c -lm -framework Foundation -framework AppKit -framework CoreVideo

#include <objc/runtime.h>
#include <objc/message.h>
#include <CoreVideo/CVDisplayLink.h>
#include <ApplicationServices/ApplicationServices.h>

#ifdef __arm64__
/* ARM just uses objc_msgSend */
#define abi_objc_msgSend_stret objc_msgSend
#define abi_objc_msgSend_fpret objc_msgSend
#else /* __i386__ */
/* x86 just uses abi_objc_msgSend_fpret and (NSColor *)objc_msgSend_id respectively */
#define abi_objc_msgSend_stret objc_msgSend_stret
#define abi_objc_msgSend_fpret objc_msgSend_fpret
#endif

typedef CGRect NSRect;
typedef CGPoint NSPoint;
typedef CGSize NSSize;

typedef void NSEvent;
typedef void NSString;
typedef void NSWindow;	
typedef void NSApplication;

typedef unsigned long NSUInteger;
typedef long NSInteger;

#define NS_ENUM(type, name) type name; enum 

typedef NS_ENUM(NSUInteger, NSWindowStyleMask) {
	NSWindowStyleMaskBorderless = 0,
	NSWindowStyleMaskTitled = 1 << 0,
	NSWindowStyleMaskClosable = 1 << 1,
	NSWindowStyleMaskMiniaturizable = 1 << 2,
	NSWindowStyleMaskResizable = 1 << 3,
	NSWindowStyleMaskTexturedBackground = 1 << 8, /* deprecated */
	NSWindowStyleMaskUnifiedTitleAndToolbar = 1 << 12,
	NSWindowStyleMaskFullScreen = 1 << 14,
	NSWindowStyleMaskFullSizeContentView = 1 << 15,
	NSWindowStyleMaskUtilityWindow = 1 << 4,
	NSWindowStyleMaskDocModalWindow = 1 << 6,
	NSWindowStyleMaskNonactivatingPanel = 1 << 7,
	NSWindowStyleMaskHUDWindow = 1 << 13
};

typedef NS_ENUM(NSUInteger, NSBackingStoreType) {
	NSBackingStoreRetained = 0,
	NSBackingStoreNonretained = 1,
	NSBackingStoreBuffered = 2
};

typedef NS_ENUM(NSUInteger, NSEventType) {        /* various types of events */
	NSEventTypeLeftMouseDown             = 1,
	NSEventTypeLeftMouseUp               = 2,
	NSEventTypeRightMouseDown            = 3,
	NSEventTypeRightMouseUp              = 4,
	NSEventTypeMouseMoved                = 5,
	NSEventTypeLeftMouseDragged          = 6,
	NSEventTypeRightMouseDragged         = 7,
	NSEventTypeMouseEntered              = 8,
	NSEventTypeMouseExited               = 9,
	NSEventTypeKeyDown                   = 10,
	NSEventTypeKeyUp                     = 11,
	NSEventTypeFlagsChanged              = 12,
	NSEventTypeAppKitDefined             = 13,
	NSEventTypeSystemDefined             = 14,
	NSEventTypeApplicationDefined        = 15,
	NSEventTypePeriodic                  = 16,
	NSEventTypeCursorUpdate              = 17,
	NSEventTypeScrollWheel               = 22,
	NSEventTypeTabletPoint               = 23,
	NSEventTypeTabletProximity           = 24,
	NSEventTypeOtherMouseDown            = 25,
	NSEventTypeOtherMouseUp              = 26,
	NSEventTypeOtherMouseDragged         = 27,
	/* The following event types are available on some hardware on 10.5.2 and later */
	NSEventTypeGesture API_AVAILABLE(macos(10.5))       = 29,
	NSEventTypeMagnify API_AVAILABLE(macos(10.5))       = 30,
	NSEventTypeSwipe   API_AVAILABLE(macos(10.5))       = 31,
	NSEventTypeRotate  API_AVAILABLE(macos(10.5))       = 18,
	NSEventTypeBeginGesture API_AVAILABLE(macos(10.5))  = 19,
	NSEventTypeEndGesture API_AVAILABLE(macos(10.5))    = 20,

	NSEventTypeSmartMagnify API_AVAILABLE(macos(10.8)) = 32,
	NSEventTypeQuickLook API_AVAILABLE(macos(10.8)) = 33,

	NSEventTypePressure API_AVAILABLE(macos(10.10.3)) = 34,
	NSEventTypeDirectTouch API_AVAILABLE(macos(10.10)) = 37,

	NSEventTypeChangeMode API_AVAILABLE(macos(10.15)) = 38,
};

typedef NS_ENUM(unsigned long long, NSEventMask) { /* masks for the types of events */
	NSEventMaskLeftMouseDown         = 1ULL << NSEventTypeLeftMouseDown,
	NSEventMaskLeftMouseUp           = 1ULL << NSEventTypeLeftMouseUp,
	NSEventMaskRightMouseDown        = 1ULL << NSEventTypeRightMouseDown,
	NSEventMaskRightMouseUp          = 1ULL << NSEventTypeRightMouseUp,
	NSEventMaskMouseMoved            = 1ULL << NSEventTypeMouseMoved,
	NSEventMaskLeftMouseDragged      = 1ULL << NSEventTypeLeftMouseDragged,
	NSEventMaskRightMouseDragged     = 1ULL << NSEventTypeRightMouseDragged,
	NSEventMaskMouseEntered          = 1ULL << NSEventTypeMouseEntered,
	NSEventMaskMouseExited           = 1ULL << NSEventTypeMouseExited,
	NSEventMaskKeyDown               = 1ULL << NSEventTypeKeyDown,
	NSEventMaskKeyUp                 = 1ULL << NSEventTypeKeyUp,
	NSEventMaskFlagsChanged          = 1ULL << NSEventTypeFlagsChanged,
	NSEventMaskAppKitDefined         = 1ULL << NSEventTypeAppKitDefined,
	NSEventMaskSystemDefined         = 1ULL << NSEventTypeSystemDefined,
	NSEventMaskApplicationDefined    = 1ULL << NSEventTypeApplicationDefined,
	NSEventMaskPeriodic              = 1ULL << NSEventTypePeriodic,
	NSEventMaskCursorUpdate          = 1ULL << NSEventTypeCursorUpdate,
	NSEventMaskScrollWheel           = 1ULL << NSEventTypeScrollWheel,
	NSEventMaskTabletPoint           = 1ULL << NSEventTypeTabletPoint,
	NSEventMaskTabletProximity       = 1ULL << NSEventTypeTabletProximity,
	NSEventMaskOtherMouseDown        = 1ULL << NSEventTypeOtherMouseDown,
	NSEventMaskOtherMouseUp          = 1ULL << NSEventTypeOtherMouseUp,
	NSEventMaskOtherMouseDragged     = 1ULL << NSEventTypeOtherMouseDragged,
};
/* The following event masks are available on some hardware on 10.5.2 and later */
#define NSEventMaskGesture API_AVAILABLE(macos(10.5))          (1ULL << NSEventTypeGesture)
#define NSEventMaskMagnify API_AVAILABLE(macos(10.5))          (1ULL << NSEventTypeMagnify)
#define NSEventMaskSwipe API_AVAILABLE(macos(10.5))            (1ULL << NSEventTypeSwipe)
#define NSEventMaskRotate API_AVAILABLE(macos(10.5))           (1ULL << NSEventTypeRotate)
#define NSEventMaskBeginGesture API_AVAILABLE(macos(10.5))     (1ULL << NSEventTypeBeginGesture)
#define NSEventMaskEndGesture API_AVAILABLE(macos(10.5))       (1ULL << NSEventTypeEndGesture)

/* Note: You can only use these event masks on 64 bit. In other words, you cannot setup a local, nor global, event monitor for these event types on 32 bit. Also, you cannot search the event queue for them (nextEventMatchingMask:...) on 32 bit. */
#define NSEventMaskSmartMagnify API_AVAILABLE(macos(10.8)) (1ULL << NSEventTypeSmartMagnify)
#define NSEventMaskPressure API_AVAILABLE(macos(10.10.3)) (1ULL << NSEventTypePressure)
#define NSEventMaskDirectTouch API_AVAILABLE(macos(10.12.2)) (1ULL << NSEventTypeDirectTouch)
#define NSEventMaskChangeMode API_AVAILABLE(macos(10.15)) (1ULL << NSEventTypeChangeMode)
#define NSEventMaskAny              NSUIntegerMax

typedef NS_ENUM(NSUInteger, NSEventModifierFlags) {
	NSEventModifierFlagCapsLock           = 1 << 16, // Set if Caps Lock key is pressed.
	NSEventModifierFlagShift              = 1 << 17, // Set if Shift key is pressed.
	NSEventModifierFlagControl            = 1 << 18, // Set if Control key is pressed.
	NSEventModifierFlagOption             = 1 << 19, // Set if Option or Alternate key is pressed.
	NSEventModifierFlagCommand            = 1 << 20, // Set if Command key is pressed.
	NSEventModifierFlagNumericPad         = 1 << 21, // Set if any key in the numeric keypad is pressed.
	NSEventModifierFlagHelp               = 1 << 22, // Set if the Help key is pressed.
	NSEventModifierFlagFunction           = 1 << 23, // Set if any function key is pressed.
};

#define objc_msgSend_id				((id (*)(id, SEL))objc_msgSend)
#define objc_msgSend_id_id			((id (*)(id, SEL, id))objc_msgSend)
#define objc_msgSend_id_rect		((id (*)(id, SEL, NSRect))objc_msgSend)
#define objc_msgSend_uint			((NSUInteger (*)(id, SEL))objc_msgSend)
#define objc_msgSend_int			((NSInteger (*)(id, SEL))objc_msgSend)
#define objc_msgSend_SEL			((SEL (*)(id, SEL))objc_msgSend)
#define objc_msgSend_float			((CGFloat (*)(id, SEL))abi_objc_msgSend_fpret)
#define objc_msgSend_bool			((BOOL (*)(id, SEL))objc_msgSend)
#define objc_msgSend_void			((void (*)(id, SEL))objc_msgSend)
#define objc_msgSend_double			((double (*)(id, SEL))objc_msgSend)
#define objc_msgSend_void_id		((void (*)(id, SEL, id))objc_msgSend)
#define objc_msgSend_void_uint		((void (*)(id, SEL, NSUInteger))objc_msgSend)
#define objc_msgSend_void_int		((void (*)(id, SEL, NSInteger))objc_msgSend)
#define objc_msgSend_void_bool		((void (*)(id, SEL, BOOL))objc_msgSend)
#define objc_msgSend_void_float		((void (*)(id, SEL, CGFloat))objc_msgSend)
#define objc_msgSend_void_double	((void (*)(id, SEL, double))objc_msgSend)
#define objc_msgSend_void_SEL		((void (*)(id, SEL, SEL))objc_msgSend)
#define objc_msgSend_id_char_const	((id (*)(id, SEL, char const *))objc_msgSend)

typedef enum NSApplicationActivationPolicy {
	NSApplicationActivationPolicyRegular,
	NSApplicationActivationPolicyAccessory,
	NSApplicationActivationPolicyProhibited
} NSApplicationActivationPolicy;

#define NSAlloc(nsclass) objc_msgSend_id((id)nsclass, sel_registerName("alloc"))
#define NSRelease(nsclass) objc_msgSend_id((id)nsclass, sel_registerName("release"))

bool running = true;

unsigned int onClose(void* self) {
	NSWindow* win = NULL;
	object_getInstanceVariable(self, "NSWindow", (void*)&win);
	if (win == NULL)
		return true;

	running = false;

	return true;
}

NSSize windowResize(void* self, SEL sel, NSSize frameSize) {
	NSWindow* win = NULL;
	object_getInstanceVariable(self, "NSWindow", (void*)&win);
	if (win == NULL)
		return frameSize;
	
	printf("window resized to %f %f\n", frameSize.width, frameSize.height);
	return frameSize;
}



#include <string.h>

const char* NSEventTypeToChar(NSEventType eventType);
const char* NSEventModifierFlagsToChar(NSEventModifierFlags modifierFlags);

int main(int argc, char* argv[]) {
	class_addMethod(objc_getClass("NSObject"), sel_registerName("windowShouldClose:"), (IMP) onClose, 0);

	NSApplication* NSApp = objc_msgSend_id((id)objc_getClass("NSApplication"), sel_registerName("sharedApplication"));
	objc_msgSend_void_int(NSApp, sel_registerName("setActivationPolicy:"), NSApplicationActivationPolicyRegular);

	NSBackingStoreType macArgs = NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSBackingStoreBuffered | NSWindowStyleMaskTitled | NSWindowStyleMaskResizable;

	SEL func = sel_registerName("initWithContentRect:styleMask:backing:defer:");
	
	NSWindow* window = ((id (*)(id, SEL, NSRect, NSWindowStyleMask, NSBackingStoreType, bool))objc_msgSend)
			(NSAlloc(objc_getClass("NSWindow")), func, 
						(NSRect){{200, 200}, {200, 200}}, 
						macArgs, macArgs, false);

	Class delegateClass = objc_allocateClassPair(objc_getClass("NSObject"), "WindowDelegate", 0);
	
	class_addIvar(
		delegateClass, "NSWindow",
		sizeof(NSWindow*), rint(log2(sizeof(NSWindow*))),
		"L"
	);

	class_addMethod(delegateClass, sel_registerName("windowWillResize:toSize:"), (IMP) windowResize, "{NSSize=ff}@:{NSSize=ff}");

	id delegate = objc_msgSend_id(NSAlloc(delegateClass), sel_registerName("init"));

	object_setInstanceVariable(delegate, "NSWindow", window);

	objc_msgSend_void_id(window, sel_registerName("setDelegate:"), delegate);

	objc_msgSend_void_bool(NSApp, sel_registerName("activateIgnoringOtherApps:"), true);
	((id(*)(id, SEL, SEL))objc_msgSend)(window, sel_registerName("makeKeyAndOrderFront:"), NULL);
	objc_msgSend_void_bool(window, sel_registerName("setIsVisible:"), true);

	objc_msgSend_void(NSApp, sel_registerName("finishLaunching"));

	while (running) {
		id pool = objc_msgSend_id(NSAlloc(objc_getClass("NSAutoreleasePool")), sel_registerName("init"));

		NSEvent* e = (NSEvent*) ((id(*)(id, SEL, NSEventMask, void*, NSString*, bool))objc_msgSend) (NSApp, sel_registerName("nextEventMatchingMask:untilDate:inMode:dequeue:"), ULONG_MAX, NULL, ((id(*)(id, SEL, const char*))objc_msgSend) ((id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), "kCFRunLoopDefaultMode"), true);


		
		unsigned int type = objc_msgSend_uint(e, sel_registerName("type"));  
		
		NSPoint p = ((NSPoint(*)(id, SEL)) objc_msgSend)(e, sel_registerName("locationInWindow"));
		
		if (type != 0)	
			printf("Event [type=%s location={%f, %f} modifierFlags={%s}]\n", 
								NSEventTypeToChar(type), 
								p.x, p.y, 
								NSEventModifierFlagsToChar(objc_msgSend_uint(e, sel_registerName("modifierFlags"))));

		objc_msgSend_void_id(NSApp, sel_registerName("sendEvent:"), e);
		((void(*)(id, SEL))objc_msgSend)(NSApp, sel_registerName("updateWindows"));
  	
		NSRelease(pool);
	}
}

const char* NSEventTypeToChar(NSEventType eventType) {
  	switch (eventType) {
		case NSEventTypeLeftMouseDown: return "LeftMouseDown";
		case NSEventTypeLeftMouseUp: return "LeftMouseUp";
		case NSEventTypeRightMouseDown: return "RightMouseDown";
		case NSEventTypeRightMouseUp: return "RightMouseUp";
		case NSEventTypeMouseMoved: return "MouseMoved";
		case NSEventTypeLeftMouseDragged: return "LeftMouseDragged";
		case NSEventTypeRightMouseDragged: return "RightMouseDragged";
		case NSEventTypeMouseEntered: return "MouseEntered";
		case NSEventTypeMouseExited: return "MouseExited";
		case NSEventTypeKeyDown: return "KeyDown";
		case NSEventTypeKeyUp: return "KeyUp";
		case NSEventTypeFlagsChanged: return "FlagsChanged";
		case NSEventTypeAppKitDefined: return "AppKitDefined";
		case NSEventTypeSystemDefined: return "SystemDefined";
		case NSEventTypeApplicationDefined: return "ApplicationDefined";
		case NSEventTypePeriodic: return "Periodic";
		case NSEventTypeCursorUpdate: return "CursorUpdate";
		case NSEventTypeScrollWheel: return "ScrollWheel";
		case NSEventTypeTabletPoint: return "TabletPoint";
		case NSEventTypeTabletProximity: return "TabletProximity";
		case NSEventTypeOtherMouseDown: return "OtherMouseDown";
		case NSEventTypeOtherMouseUp: return "OtherMouseUp";
		case NSEventTypeOtherMouseDragged: return "OtherMouseDragged";
		default: return "N/A";
 	}
}

char* ns_strcat(register char *s, register const char *append) {
	char *save = s;

	for (; *s; ++s);
	while ((*s++ = *append++));
	return save;
}

const char* NSEventModifierFlagsToChar(NSEventModifierFlags modifierFlags) {
	static char result[100];
	result[0] = '\0';

	if ((modifierFlags & NSEventModifierFlagCapsLock) == NSEventModifierFlagCapsLock) ns_strcat(result, "CapsLock, ");
	if ((modifierFlags & NSEventModifierFlagShift) == NSEventModifierFlagShift) ns_strcat(result, "NShift, ");
	if ((modifierFlags & NSEventModifierFlagControl) == NSEventModifierFlagControl) ns_strcat(result, "Control, ");
	if ((modifierFlags & NSEventModifierFlagOption) == NSEventModifierFlagOption) ns_strcat(result, "Option, ");
	if ((modifierFlags & NSEventModifierFlagCommand) == NSEventModifierFlagCommand) ns_strcat(result, "Command, ");
	if ((modifierFlags & NSEventModifierFlagNumericPad) == NSEventModifierFlagNumericPad) ns_strcat(result, "NumericPad, ");
	if ((modifierFlags & NSEventModifierFlagHelp) == NSEventModifierFlagHelp) ns_strcat(result, "Help, ");
	if ((modifierFlags & NSEventModifierFlagFunction) == NSEventModifierFlagFunction) ns_strcat(result, "Function, ");

	return result;
}

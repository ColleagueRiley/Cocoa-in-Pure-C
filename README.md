# RGFW Under the Hood: Cocoa in Pure C

## Introduction

To use Apple's Cocoa API, you must use Objective-C function calls. However, you do not have to write Objective-C code because Objective-C can be accessed via C functions such as  `objc_msgSend`.

The main reason to use Pure-C over Objective-C is to be able to compile your project in C. This is helpful if you want to create a single-header file that does not require the user to compile using Objective-C.

Two examples of this are: 

[Silicon.h](https://github.com/EimaMei/Silicon), a C-Wrapper for the Cocoa API that wraps around the Objective-C function calls so you can use the Cocoa API in normal-looking C code, and [RGFW](https://github.com/ColleagueRiley/RGFW), a lightweight single-header windowing library. 

Both projects can be used as a reference for using Cocoa in C.

## Overview
A quick overview of the topics the article will cover

1) The Basics of Using Objective-C in Pure C 
2) Defining Cocoa Types
3) Creating a Basic Cocoa Window 

## 1. The Basics of using Objective-C in Pure C
Objective-C functions can be called using [`objc_msgsend`](https://developer.apple.com/documentation/objectivec/1456712-objc_msgsend).

Due to ABI differences, ARM uses `objc_msgsend` for all cases. However `x86_64` CPUs require the use of specific functions for floating point and structure returns. 
[`objc_msgsend_fpret`](https://developer.apple.com/documentation/objectivec/1456697-objc_msgsend_fpret) for functions with floating point returns and 
[`objc_msgsend_fstret`](https://developer.apple.com/documentation/objectivec/1456730-objc_msgsend_stret) for functions that return a structure. 

RGFW handles this like this:

```c
#include <objc/runtime.h>
#include <objc/message.h> 

#ifdef __arm64__
/* ARM just uses objc_msgSend */
#define abi_objc_msgSend_stret objc_msgSend
#define abi_objc_msgSend_fpret objc_msgSend
#else /* __i386__ */
/* x86 just uses abi_objc_msgSend_fpret and (NSColor *)objc_msgSend_id respectively */
#define abi_objc_msgSend_stret objc_msgSend_stret
#define abi_objc_msgSend_fpret objc_msgSend_fpret
#endif
```

`objc_msgSend` is a generic function, so its type must be cast based on the return and argument types you want.

For example: `((int (*)(id, SEL, int))objc_msgSend)` for a function that takes an int argument and returns an int. 

To avoid repeating commonly used type-casting, RGFW defines macros to handle common cases. 

```c
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
```

You might notice two common arguments in these functions, `id` and `SEL`

The [`id`](https://developer.apple.com/documentation/objectivec/id) argument refers to an ID of an Objective-C object or class. 

[`SEL`](https://developer.apple.com/documentation/objectivec/sel) refers to the function's selector. 

For example `MyObject *bar = [MyObject objectWithString:@"RGFW"];` can be translated to `id* bar = [id SEL:@"RGFW"];`


To get the ID to an Objective-C class you must run [`objc_getClass`](https://developer.apple.com/documentation/objectivec/1418952-objc_getclass)

For example: `objc_getClass("NSWindow");`

To get the selector for an Objective-C function you must use [`sel_registerName`](https://developer.apple.com/documentation/objectivec/1418557-sel_registername)

The syntax of Objective-C functions is like this `<function-name>`, then `:` is used as a placeholder for an argument.

A function with one argument would look like this:

`sel_registerName("makeKeyAndOrderFront:");`

However, a function with no arguments would look like this.

`sel_registerName("isKeyWindow");`

If the function has multiple arguments you will have to also add the argument names (not including the first argument)

`sel_registerName("initWithContentRect:styleMask:backing:defer:");`

To define a class method (eg. a callback function for the object) you must use [`class_addMethod`](https://developer.apple.com/documentation/objectivec/1418901-class_addmethod) this function takes the [delegate class](https://developer.apple.com/documentation/uikit/uisceneconfiguration/3197948-delegateclass?changes=_4) (class of the object that is calling's [delegate](https://developer.apple.com/library/archive/documentation/General/Conceptual/CocoaEncyclopedia/DelegatesandDataSources/DelegatesandDataSources.html) 
), selector of the function being called, the function you want to be called, and the arguments expected in string format.

But first, you must allocate the delegate class to access it.
You can do this using [`objc_allocateClassPair`](https://developer.apple.com/documentation/objectivec/1418559-objc_allocateclasspair).

For example to allocate the delegate class for an NSWindow: 

```c
Class delegateClass = objc_allocateClassPair(objc_getClass("NSObject"), "WindowDelegate", 0);
```

To create a call back you have to use [`class_addMethod`](https://developer.apple.com/documentation/objectivec/1418901-class_addmethod) to set it as the callback for the class.

Creating  a call back for NSWindow's [`windowWillResize`](https://developer.apple.com/documentation/appkit/nswindowdelegate/1419292-windowwillresize) would look like this:

```c
class_addMethod(delegateClass, sel_registerName("windowWillResize:toSize:"), (IMP) windowResize, "{NSSize=ff}@:{NSSize=ff}");
```

You can also add custom user variables to the class, these can be used to attach user data to the class.

For example, RGFW attaches an `RGFW_window` to the NSWindow class to modify the `RGFW_window*` data. This can be done using [`class_addIvar`](https://developer.apple.com/documentation/objectivec/1418756-class_addivar)

```c
class_addIvar(
	delegateClass, "RGFW_window",
	sizeof(RGFW_window*), rint(log2(sizeof(RGFW_window*))),
	"L"	
);
```

To set the variable to use for your NSWindow instance, use 

[`object_setInstanceVariable`](https://developer.apple.com/documentation/objectivec/1441498-object_setinstancevariable)

To set the `RGFW_window` object to be the variable instance for its NSWindow object:

`object_setInstanceVariable(delegate, "NSWindow", window);`

You can get the instance variable of an object via [`object_getInstanceVariable`](https://developer.apple.com/documentation/objectivec/1441499-object_getinstancevariable?changes=__9&language=objc)

```c
RGFW_window* win = NULL;

//The object variable would be called "self" in a callback
object_getInstanceVariable(self, "RGFW_window", (void*)&win);
```

## Defining Cocoa Types
The Cocoa header files are written in Objective-C. This means we'll have to define the types and enums ourselves.

The shape types can be defined as the CG Shapes like this:

```c
typedef CGRect NSRect;
typedef CGPoint NSPoint;
typedef CGSize NSSize;
```

Cocoa also uses custom integer-type names,
these can be defined to be their matching c datatype.

```c
typedef unsigned long NSUInteger;
typedef long NSInteger;
```

You can also define Cocoa objects. I define them as void so I can use them as pointers.

```c
// Note: void is being used here 
// type* can be used ( type* == void* == id ) 
typedef void NSEvent;
typedef void NSString;
typedef void NSWindow;	
typedef void NSApplication;
```


As for the enums, here are some of the enums I will be using in this tutorial.

Many of the other enums can be found in Cocoa's headers, documentation, RGFW.h, or Silicon. h.

```c
/* this macro is used to give an enum a type */ 
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
```

RGFW also defines [`NSAlloc`](https://developer.apple.com/documentation/objectivec/nsobject/1571958-alloc) and [`NSRelease`](https://developer.apple.com/documentation/objectivec/1418956-nsobject/1571957-release) because they're basic functions that are used a lot.

```c
#define NSAlloc(nsclass) objc_msgSend_id((id)nsclass, sel_registerName("alloc"))
#define NSRelease(nsclass) objc_msgSend_id((id)nsclass, sel_registerName("release"))
```


## 3. Creating a Basic Cocoa Window

Now that you understand the basics of calling Objective-C functions from C, and the setup required to use Cocoa, it's time to apply that for creating a basic window using Cocoa. 

First, some library headers are required. 

```c
#include <CoreVideo/CVDisplayLink.h> // !
#include <ApplicationServices/ApplicationServices.h>

#include <string.h>
```

These functions will be used for printing information about the current event.

```c
const char* NSEventTypeToChar(NSEventType eventType);
const char* NSEventModifierFlagsToChar(NSEventModifierFlags modifierFlags);

// These will be placed after the main function and will be defined like this:
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
```

Cocoa does not handle certain events using the event loop. 
Instead, they're fed to the NSWindow via callbacks.

Here's how these functions will be defined for this example:

```c
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
```


The first thing that I will do in the main function is define the [`windowShouldClose`](https://developer.apple.com/documentation/appkit/nswindowdelegate/1419380-windowshouldclose) callback.

This is so that way the program doesn't keep running after the window is closed.

```c
class_addMethod(objc_getClass("NSObject"), sel_registerName("windowShouldClose:"), (IMP) onClose, 0);
```

Next, the [NSApplication](https://developer.apple.com/documentation/appkit/nsapplication) is set up.

This requires the use of [`sharedApplication`](https://developer.apple.com/documentation/appkit/nsapplication/1428360-sharedapplication) and [`setActivationPolicy`](https://developer.apple.com/documentation/appkit/nsapplication/1428621-setactivationpolicy)

```c
NSApplication* NSApp = objc_msgSend_id((id)objc_getClass("NSApplication"), sel_registerName("sharedApplication"));
objc_msgSend_void_id(NSApp, sel_registerName("setActivationPolicy:"), NSApplicationActivationPolicyRegular);
```

Now you can create a [NSWindow](https://developer.apple.com/documentation/appkit/nswindow) can be created, I broke the window creation process into three steps so it's more readable.

The window is created using [`initWithContentRect`](https://developer.apple.com/documentation/appkit/nswindow/1419477-init)

```c
NSBackingStoreType macArgs = NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSBackingStoreBuffered | NSWindowStyleMaskTitled | NSWindowStyleMaskResizable;

SEL func = sel_registerName("initWithContentRect:styleMask:backing:defer:");
	
NSWindow* window = ((id (*)(id, SEL, NSRect, NSWindowStyleMask, NSBackingStoreType, bool))objc_msgSend)
			(NSAlloc(objc_getClass("NSWindow")), func, 
						(NSRect){{200, 200}, {200, 200}}, 
						macArgs, macArgs, false);
```


You can then set up the delegate class and window resize callback.

```c
Class delegateClass = objc_allocateClassPair(objc_getClass("NSObject"), "WindowDelegate", 0);
	
class_addIvar(
	    delegateClass, "NSWindow",
		sizeof(NSWindow*), rint(log2(sizeof(NSWindow*))),
		"L"
);

class_addMethod(delegateClass, sel_registerName("windowWillResize:toSize:"), (IMP) windowResize, "{NSSize=ff}@:{NSSize=ff}");
```

After that, the delegate must be initialized using [`init`](https://developer.apple.com/documentation/objectivec/nsobject/1418641-init)

Then I will set the delegate's variable data as our NSWindow and set the NSWindow's delegate to be the delegate we initialized using [`setDelegate`](https://developer.apple.com/documentation/foundation/nsmachport/1399547-setdelegate)

```c
id delegate = objc_msgSend_id(NSAlloc(delegateClass), sel_registerName("init"));

object_setInstanceVariable(delegate, "NSWindow", window);

objc_msgSend_void_id(window, sel_registerName("setDelegate:"), delegate);
```

Then the window can be shown using [`setIsVisible`](https://developer.apple.com/documentation/appkit/nswindow/1449570-setisvisible) and made key and front via [`makeKeyAndOrderFront`](https://developer.apple.com/documentation/appkit/nswindow/1419208-makekeyandorderfront), this puts it in focus.

activatIgnoredOtherApps is required to open the window 

```c
objc_msgSend_void_bool(NSApp, sel_registerName("activateIgnoringOtherApps:"), true);
((id(*)(id, SEL, SEL))objc_msgSend)(window, sel_registerName("makeKeyAndOrderFront:"), NULL);
objc_msgSend_void_bool(window, sel_registerName("setIsVisible:"), true);

objc_msgSend_void(NSApp, sel_registerName("finishLaunching"));
```

Now, in the draw loop, I'd start by creating a memory pool.

This is so that way all memory allocated by event checking can be freed at once, avoiding a memory leak.

This can be done using [`NSAutoreleasePool`](https://developer.apple.com/documentation/foundation/nsautoreleasepool)

```c
id pool = objc_msgSend_id(NSAlloc(objc_getClass("NSAutoreleasePool")), sel_registerName("init"));
```


Now the current event can be checked using an [`NSEvent`](https://developer.apple.com/documentation/appkit/nsevent) object and [`nextEventMatchingMask`](https://developer.apple.com/documentation/appkit/nsapplication/1428485-nexteventmatchingmask)

The event type can be found using [`type`](https://developer.apple.com/documentation/appkit/nsevent/1528439-type)
The event mouse point can be found using [`locationInWindow`](https://developer.apple.com/documentation/appkit/nsevent/1529068-locationinwindow)
The event modifier flags can be found using [`modifierFlags`](https://developer.apple.com/documentation/appkit/nsevent/1534405-modifierflags)

```c
NSEvent* e = (NSEvent*) ((id(*)(id, SEL, NSEventMask, void*, NSString*, bool))objc_msgSend) (NSApp, sel_registerName("nextEventMatchingMask:untilDate:inMode:dequeue:"), ULONG_MAX, NULL,                           ((id(*)(id, SEL, const char*))objc_msgSend) ((id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), "kCFRunLoopDefaultMode"), true);

unsigned int type = objc_msgSend_uint(e, sel_registerName("type"));  

NSPoint p = ((NSPoint(*)(id, SEL)) objc_msgSend)(e, sel_registerName("locationInWindow"));
```

Before I check the event, I make sure there is an event.


```c
if (type == 0)
    printf("Event [type=%s location={%f, %f} modifierFlags={%s}]\n", 
                        NSEventTypeToChar(type), 
                        p.x, p.y, 
                        NSEventModifierFlagsToChar(objc_msgSend_uint(e, sel_registerName("modifierFlags"))));

```

The event can be pushed out using [`sendEvent`](https://developer.apple.com/documentation/uikit/uiapplication/1623043-sendevent) and the window can be updated using [`updateWindows`](https://developer.apple.com/documentation/appkit/nsapplication/1428675-updatewindows)

```c
objc_msgSend_void_id(NSApp, sel_registerName("sendEvent:"), e);
((void(*)(id, SEL))objc_msgSend)(NSApp, sel_registerName("updateWindows"));
```

At the end of the draw loop, the event pool should be freed.

```c
NSRelease(pool);
```

## Full code example
```c
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
```

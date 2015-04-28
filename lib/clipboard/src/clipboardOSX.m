
#include <stdbool.h>
#include <string.h>

#include <AppKit/AppKit.h>

char *widgetGetClipboardText()
{
	NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
	NSString *myString = [pasteboard stringForType:NSPasteboardTypeString];

	if (myString == nil || [myString length] == 0) return NULL;

	return strdup([myString UTF8String]);
}

bool widgetSetClipboardText(const char *text)
{
	NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
	[pasteboard clearContents];

	NSString *stringFromUTFString = [[NSString alloc] initWithUTF8String:text];
	return [pasteboard setString:stringFromUTFString forType:NSStringPboardType];
}

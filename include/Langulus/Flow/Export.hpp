///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include <Langulus/Core.hpp>
#include <Langulus/TTag.hpp>

#if defined(LANGULUS_EXPORT_ALL) || defined(LANGULUS_EXPORT_FLOW)
   #define LANGULUS_API_FLOW() LANGULUS_EXPORT()
#else
   #define LANGULUS_API_FLOW() LANGULUS_IMPORT()
#endif

/// Make the rest of the code aware, that Langulus::Flow has been included    
#define LANGULUS_LIBRARY_FLOW() 1


LANGULUS_DEFINE_TAG(Logger,
   "Logger instances");
LANGULUS_DEFINE_TAG(Count,
   "Container size, or other similar properties");
LANGULUS_DEFINE_TAG(Name,
   "Names, or other similar properties");
LANGULUS_DEFINE_TAG(Path,
   "Files, folders, or other file-system related things");
LANGULUS_DEFINE_TAG(Data,
   "Raw container data, or other similar properties");
LANGULUS_DEFINE_TAG(Index,
   "Index of an element in a container, or other similar properties");
LANGULUS_DEFINE_TAG(Context,
   "Verb source, code environment, or other similar properties");
LANGULUS_DEFINE_TAG(Tag,
   "Variables and properties");
LANGULUS_DEFINE_TAG(State,
   "Boolean states and other similar properties");
LANGULUS_DEFINE_TAG(Child,
   "Children in a hierarchy");
LANGULUS_DEFINE_TAG(Parent,
   "Parents in a hierarchy");
LANGULUS_DEFINE_TAG(Clipboard,
   "Operating system clipboard");
LANGULUS_DEFINE_TAG(Color,
   "Color properties");
LANGULUS_DEFINE_TAG(Input,
   "Verb arguments, or general inputs of an operation");
LANGULUS_DEFINE_TAG(Output,
   "Verb outputs, or general output of an operation");
LANGULUS_DEFINE_TAG(Mass,
   "Magnitude, amplitude, or just physical mass");
LANGULUS_DEFINE_TAG(Rate,
   "Rate, frequency, or other similar properties");
LANGULUS_DEFINE_TAG(Time,
   "Temporal properties");
LANGULUS_DEFINE_TAG(Precedence,
   "Precedence, priority, or other similar properties");
LANGULUS_DEFINE_TAG(Runtime,
   "Accesses the runtime of a hierarchy of Things");
LANGULUS_DEFINE_TAG(Part,
   "Accesses parts (components) of Things");
LANGULUS_DEFINE_TAG(Mesh,
   "Associated geometry art");
LANGULUS_DEFINE_TAG(FOV,
   "Horizontal field of view angle, usually a real number");
LANGULUS_DEFINE_TAG(AspectRatio,
   "Aspect ratio (width / height), usually a real number");
LANGULUS_DEFINE_TAG(Viewport,
   "Viewport and depth clipping, usually a Range4");
LANGULUS_DEFINE_TAG(Image, 
   "Associated image art");
LANGULUS_DEFINE_TAG(Material,
   "Associated material art");
LANGULUS_DEFINE_TAG(Shader, 
   "Associated shader art");
LANGULUS_DEFINE_TAG(Cursor,
   "Enables or disables cursor for window");
LANGULUS_DEFINE_TAG(Monitor,
   "Monitor to which window is bound, making it fullscreen");
LANGULUS_DEFINE_TAG(MousePosition,
   "Mouse position, relative to a window");
LANGULUS_DEFINE_TAG(MouseScroll,
   "Mouse scroll for window");
LANGULUS_DEFINE_TAG(NativeWindowHandle,
   "Native window handle");
LANGULUS_DEFINE_TAG(Modal,
   "Most occuring value in a set");
LANGULUS_DEFINE_TAG(Smallest,
   "The smallest value in a set");
LANGULUS_DEFINE_TAG(Largest,
   "The biggest value in a set");
LANGULUS_DEFINE_TAG(Range,
   "Context dependent range");


// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_text_TabableView__
#define __javax_swing_text_TabableView__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace javax
  {
    namespace swing
    {
      namespace text
      {
          class TabExpander;
          class TabableView;
      }
    }
  }
}

class javax::swing::text::TabableView : public ::java::lang::Object
{

public:
  virtual jfloat getPartialSpan(jint, jint) = 0;
  virtual jfloat getTabbedSpan(jfloat, ::javax::swing::text::TabExpander *) = 0;
  static ::java::lang::Class class$;
} __attribute__ ((java_interface));

#endif // __javax_swing_text_TabableView__
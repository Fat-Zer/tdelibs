/* This file is part of the KDE libraries
    Copyright (C) 1998 J�rg Habenicht (j.habenicht@europemail.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#define PAINT_BENCH
#undef PAINT_BENCH

#ifdef PAINT_BENCH
#include <tqdatetime.h>
#include <stdio.h>
#endif


#include <tqpainter.h>
#include <tqimage.h>
#include <tqcolor.h>
#include <kapplication.h>
#include <kpixmapeffect.h>
#include "kled.h"


class KLed::KLedPrivate
{
  friend class KLed;

  int dark_factor;
  TQColor offcolor;
  TQPixmap *off_map;
  TQPixmap *on_map;
};



KLed::KLed(TQWidget *parent, const char *name)
  : TQWidget( parent, name),
    led_state(On),
    led_look(Raised),
    led_tqshape(Circular)
{
  TQColor col(green);
  d = new KLed::KLedPrivate;
  d->dark_factor = 300;
  d->offcolor = col.dark(300);
  d->off_map = 0;
  d->on_map = 0;

  setColor(col);
}


KLed::KLed(const TQColor& col, TQWidget *parent, const char *name)
  : TQWidget( parent, name),
    led_state(On),
    led_look(Raised),
    led_tqshape(Circular)
{
  d = new KLed::KLedPrivate;
  d->dark_factor = 300;
  d->offcolor = col.dark(300);
  d->off_map = 0;
  d->on_map = 0;

  setColor(col);
  //setShape(Circular);
}

KLed::KLed(const TQColor& col, KLed::State state,
	   KLed::Look look, KLed::Shape tqshape, TQWidget *parent, const char *name )
  : TQWidget(parent, name),
    led_state(state),
    led_look(look),
    led_tqshape(tqshape)
{
  d = new KLed::KLedPrivate;
  d->dark_factor = 300;
  d->offcolor = col.dark(300);
  d->off_map = 0;
  d->on_map = 0;

  //setShape(tqshape);
  setColor(col);
}


KLed::~KLed()
{
  delete d->off_map;
  delete d->on_map;
  delete d;
}

void
KLed::paintEvent(TQPaintEvent *)
{
#ifdef PAINT_BENCH
  const int rounds = 1000;
  TQTime t;
  t.start();
  for (int i=0; i<rounds; i++) {
#endif
  switch(led_tqshape)
    {
    case Rectangular:
      switch (led_look)
	{
	case Sunken :
	  paintRectFrame(false);
	  break;
	case Raised :
	  paintRectFrame(true);
	  break;
	case Flat   :
	  paintRect();
	  break;
	default  :
	  qWarning("%s: in class KLed: no KLed::Look set",tqApp->argv()[0]);
	}
      break;
    case Circular:
      switch (led_look)
	{
	case Flat   :
	  paintFlat();
	  break;
	case Raised :
	  paintRound();
	  break;
	case Sunken :
	  paintSunken();
	  break;
	default:
	  qWarning("%s: in class KLed: no KLed::Look set",tqApp->argv()[0]);
	}
      break;
    default:
      qWarning("%s: in class KLed: no KLed::Shape set",tqApp->argv()[0]);
      break;
    }
#ifdef PAINT_BENCH
  }
  int ready = t.elapsed();
  qWarning("elapsed: %d msec. for %d rounds", ready, rounds);
#endif
}

int
KLed::ensureRoundLed()
{
    // Initialize coordinates, width, and height of the LED
    //
    int width = this->width();
    // Make sure the LED is round!
    if (width > this->height())
        width = this->height();
    width -= 2; // leave one pixel border
    if (width < 0)
        width = 0;

    return width;
}

bool
KLed::paintCachedPixmap()
{
    if (led_state) {
        if (d->on_map) {
            TQPainter paint(this);
            paint.drawPixmap(0, 0, *d->on_map);
            return true;
        }
    } else {
        if (d->off_map) {
            TQPainter paint(this);
            paint.drawPixmap(0, 0, *d->off_map);
            return true;
        }
    }

    return false;
}

void
KLed::paintFlat() // paint a ROUND FLAT led lamp
{
    if (paintCachedPixmap()) return;

    TQPainter paint;
    TQColor color;
    TQBrush brush;
    TQPen pen;

    int width = ensureRoundLed();


    int scale = 3;
    TQPixmap *tmpMap = 0;

    width *= scale;

    tmpMap = new TQPixmap(width + 6, width + 6);
    tmpMap->fill(paletteBackgroundColor());

    // start painting widget
    //
    paint.begin(tmpMap);

    // Set the color of the LED according to given parameters
    color = ( led_state ) ? led_color : d->offcolor;

    // Set the brush to SolidPattern, this fills the entire area
    // of the ellipse which is drawn with a thin gray "border" (pen)
    brush.setStyle( Qt::SolidPattern );
    brush.setColor( color );

    pen.setWidth( scale );
    color = tqcolorGroup().dark();
    pen.setColor( color );			// Set the pen accordingly

    paint.setPen( pen );			// Select pen for drawing
    paint.setBrush( brush );		// Assign the brush to the painter

    // Draws a "flat" LED with the given color:
    paint.drawEllipse( scale, scale, width - scale * 2, width - scale * 2 );

    paint.end();
    //
    // painting done
    TQPixmap *&dest = led_state ? d->on_map : d->off_map;
    TQImage i = tmpMap->convertToImage();
    width /= 3;
    i = i.smoothScale(width, width);
    delete tmpMap;
    dest = new TQPixmap(i);
    paint.begin(this);
    paint.drawPixmap(0, 0, *dest);
    paint.end();

}

void
KLed::paintRound() // paint a ROUND RAISED led lamp
{
    if (paintCachedPixmap()) return;

    TQPainter paint;
    TQColor color;
    TQBrush brush;
    TQPen pen;

    // Initialize coordinates, width, and height of the LED
    int width = ensureRoundLed();

    int scale = 3;
    TQPixmap *tmpMap = 0;

    width *= scale;

    tmpMap = new TQPixmap(width + 6, width + 6);
    tmpMap->fill(paletteBackgroundColor());
    paint.begin(tmpMap);

    // Set the color of the LED according to given parameters
    color = ( led_state ) ? led_color : d->offcolor;

    // Set the brush to SolidPattern, this fills the entire area
    // of the ellipse which is drawn first
    brush.setStyle( Qt::SolidPattern );
    brush.setColor( color );
    paint.setBrush( brush );		// Assign the brush to the painter

    // Draws a "flat" LED with the given color:
    paint.drawEllipse( scale, scale, width - scale*2, width - scale*2 );

    // Draw the bright light spot of the LED now, using modified "old"
    // painter routine taken from TDEUI�s KLed widget:

    // Setting the new width of the pen is essential to avoid "pixelized"
    // shadow like it can be observed with the old LED code
    pen.setWidth( 2 * scale );

    // shrink the light on the LED to a size about 2/3 of the complete LED
    int pos = width/5 + 1;
    int light_width = width;
    light_width *= 2;
    light_width /= 3;

    // Calculate the LED�s "light factor":
    int light_quote = (130*2/(light_width?light_width:1))+100;

    // Now draw the bright spot on the LED:
    while (light_width) {
    	color = color.light( light_quote );			// make color lighter
	pen.setColor( color );				// set color as pen color
	paint.setPen( pen );				// select the pen for drawing
	paint.drawEllipse( pos, pos, light_width, light_width );	// draw the ellipse (circle)
	light_width--;
   	if (!light_width)
     		 break;
	paint.drawEllipse( pos, pos, light_width, light_width );
	light_width--;
	if (!light_width)
  		break;
	paint.drawEllipse( pos, pos, light_width, light_width );
	pos++; light_width--;
    }

    // Drawing of bright spot finished, now draw a thin gray border
    // around the LED; it looks nicer that way. We do this here to
    // avoid that the border can be erased by the bright spot of the LED

    pen.setWidth( 2 * scale + 1 );
    color = tqcolorGroup().dark();
    pen.setColor( color );			// Set the pen accordingly
    paint.setPen( pen );			// Select pen for drawing
    brush.setStyle( Qt::NoBrush );		// Switch off the brush
    paint.setBrush( brush );			// This avoids filling of the ellipse

    paint.drawEllipse( 2, 2, width, width );

    paint.end();
    //
    // painting done
    TQPixmap *&dest = led_state ? d->on_map : d->off_map;
    TQImage i = tmpMap->convertToImage();
    width /= 3;
    i = i.smoothScale(width, width);
    delete tmpMap;
    dest = new TQPixmap(i);
    paint.begin(this);
    paint.drawPixmap(0, 0, *dest);
    paint.end();

}

void
KLed::paintSunken() // paint a ROUND SUNKEN led lamp
{
    if (paintCachedPixmap()) return;

    TQPainter paint;
    TQColor color;
    TQBrush brush;
    TQPen pen;

    // First of all we want to know what area should be updated
    // Initialize coordinates, width, and height of the LED
    int	width = ensureRoundLed();

    int scale = 3;
    TQPixmap *tmpMap = 0;

    width *= scale;

    tmpMap = new TQPixmap(width, width);
    tmpMap->fill(paletteBackgroundColor());
    paint.begin(tmpMap);

    // Set the color of the LED according to given parameters
    color = ( led_state ) ? led_color : d->offcolor;

    // Set the brush to SolidPattern, this fills the entire area
    // of the ellipse which is drawn first
    brush.setStyle( Qt::SolidPattern );
    brush.setColor( color );
    paint.setBrush( brush );                // Assign the brush to the painter

    // Draws a "flat" LED with the given color:
    paint.drawEllipse( scale, scale, width - scale*2, width - scale*2 );

    // Draw the bright light spot of the LED now, using modified "old"
    // painter routine taken from TDEUI�s KLed widget:

    // Setting the new width of the pen is essential to avoid "pixelized"
    // shadow like it can be observed with the old LED code
    pen.setWidth( 2 * scale );

    // shrink the light on the LED to a size about 2/3 of the complete LED
    int pos = width/5 + 1;
    int light_width = width;
    light_width *= 2;
    light_width /= 3;

    // Calculate the LED�s "light factor":
    int light_quote = (130*2/(light_width?light_width:1))+100;

    // Now draw the bright spot on the LED:
    while (light_width) {
	color = color.light( light_quote );                      // make color lighter
	pen.setColor( color );                                   // set color as pen color
	paint.setPen( pen );                                     // select the pen for drawing
	paint.drawEllipse( pos, pos, light_width, light_width ); // draw the ellipse (circle)
	light_width--;
	if (!light_width)
		break;
	paint.drawEllipse( pos, pos, light_width, light_width );
	light_width--;
	if (!light_width)
		break;
	paint.drawEllipse( pos, pos, light_width, light_width );
	pos++; light_width--;
    }

    // Drawing of bright spot finished, now draw a thin border
    // around the LED which resembles a shadow with light coming
    // from the upper left.

    pen.setWidth( 2 * scale + 1 ); // ### shouldn't this value be smaller for smaller LEDs?
    brush.setStyle( (Qt::BrushStyle)NoBrush );              // Switch off the brush
    paint.setBrush( brush );                        // This avoids filling of the ellipse

    // Set the initial color value to colorGroup().light() (bright) and start
    // drawing the shadow border at 45� (45*16 = 720).

    int angle = -720;
    color = tqcolorGroup().light();

    for ( int arc = 120; arc < 2880; arc += 240 ) {
      pen.setColor( color );
      paint.setPen( pen );
      int w = width - pen.width()/2 - scale + 1;
      paint.drawArc( pen.width()/2, pen.width()/2, w, w, angle + arc, 240 );
      paint.drawArc( pen.width()/2, pen.width()/2, w, w, angle - arc, 240 );
      color = color.dark( 110 ); //FIXME: this should somehow use the contrast value
    }	// end for ( angle = 720; angle < 6480; angle += 160 )

    paint.end();
    //
    // painting done

    TQPixmap *&dest = led_state ? d->on_map : d->off_map;
    TQImage i = tmpMap->convertToImage();
    width /= 3;
    i = i.smoothScale(width, width);
    delete tmpMap;
    dest = new TQPixmap(i);
    paint.begin(this);
    paint.drawPixmap(0, 0, *dest);
    paint.end();

}

void
KLed::paintRect()
{
  TQPainter painter(this);
  TQBrush lightBrush(led_color);
  TQBrush darkBrush(d->offcolor);
  TQPen pen(led_color.dark(300));
  int w=width();
  int h=height();
  // -----
  switch(led_state)
  {
  case On:
    painter.setBrush(lightBrush);
    painter.drawRect(0, 0, w, h);
    break;
  case Off:
    painter.setBrush(darkBrush);
    painter.drawRect(0, 0, w, h);
    painter.setPen(pen);
    painter.drawLine(0, 0, w, 0);
    painter.drawLine(0, h-1, w, h-1);
    // Draw verticals
    int i;
    for(i=0; i < w; i+= 4 /* dx */)
      painter.drawLine(i, 1, i, h-1);
    break;
  default: break;
  }
}

void
KLed::paintRectFrame(bool raised)
{
  TQPainter painter(this);
  TQBrush lightBrush(led_color);
  TQBrush darkBrush(d->offcolor);
  int w=width();
  int h=height();
  TQColor black=Qt::black;
  TQColor white=Qt::white;
  // -----
  if(raised)
    {
      painter.setPen(white);
      painter.drawLine(0, 0, 0, h-1);
      painter.drawLine(1, 0, w-1, 0);
      painter.setPen(black);
      painter.drawLine(1, h-1, w-1, h-1);
      painter.drawLine(w-1, 1, w-1, h-1);
      painter.fillRect(1, 1, w-2, h-2,
       		       (led_state==On)? lightBrush : darkBrush);
    } else {
      painter.setPen(black);
      painter.drawRect(0,0,w,h);
      painter.drawRect(0,0,w-1,h-1);
      painter.setPen(white);
      painter.drawRect(1,1,w-1,h-1);
      painter.fillRect(2, 2, w-4, h-4,
		       (led_state==On)? lightBrush : darkBrush);
    }
}

KLed::State
KLed::state() const
{
  return led_state;
}

KLed::Shape
KLed::tqshape() const
{
  return led_tqshape;
}

TQColor
KLed::color() const
{
  return led_color;
}

KLed::Look
KLed::look() const
{
  return led_look;
}

void
KLed::setState( State state )
{
  if (led_state != state)
    {
      led_state = state;
      update();
    }
}

void
KLed::toggleState()
{
  toggle();
}

void
KLed::setShape(KLed::Shape s)
{
  if(led_tqshape!=s)
    {
      led_tqshape = s;
      update();
    }
}

void
KLed::setColor(const TQColor& col)
{
  if(led_color!=col) {
    if(d->on_map)  { delete d->on_map; d->on_map = 0; }
    if(d->off_map) { delete d->off_map; d->off_map = 0; }
    led_color = col;
    d->offcolor = col.dark(d->dark_factor);
    update();
  }
}

void
KLed::setDarkFactor(int darkfactor)
{
  if (d->dark_factor != darkfactor) {
    d->dark_factor = darkfactor;
    d->offcolor = led_color.dark(darkfactor);
    update();
  }
}

int
KLed::darkFactor() const
{
  return d->dark_factor;
}

void
KLed::setLook( Look look )
{
  if(led_look!=look)
    {
      led_look = look;
      update();
    }
}

void
KLed::toggle()
{
  led_state = (led_state == On) ? Off : On;
  // setColor(led_color);
  update();
}

void
KLed::on()
{
  setState(On);
}

void
KLed::off()
{
  setState(Off);
}

TQSize
KLed::sizeHint() const
{
  return TQSize(16, 16);
}

TQSize
KLed::minimumSizeHint() const
{
  return TQSize(16, 16 );
}

void KLed::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kled.moc"

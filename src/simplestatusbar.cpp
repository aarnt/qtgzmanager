/*
* This file is part of QTGZManager, an open-source GUI for Slackware pkgtools.
* Copyright (C) 2006  Alexandre Albuquerque Arnt
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* Contact e-mail : Alexandre Albuquerque Arnt <qtgzmanager@gmail.com>
* Program URL   : http://jtgzmanager.sf.net
*
*/

#include <iostream>
#include "simplestatusbar.h"

#include <QBoxLayout>
#include <QSizeGrip>
#include <QStyleOption>
#include <QPainter>

class SimpleStatusBarPrivate {
  //Q_DECLARE_PUBLIC(SimpleStatusBar)
public:
  SimpleStatusBarPrivate() {}
  virtual ~SimpleStatusBarPrivate() { }

	struct SBItem {
    SBItem(QWidget* widget, int stretch, bool permanent)
      : s(stretch), w(widget), p(permanent) {}
    int s;
    QWidget * w;
    bool p;
	};

	QList<SBItem *> items;
	QString tempItem;

	QBoxLayout * box;
	QTimer * timer;

#ifndef QT_NO_SIZEGRIP
	QSizeGrip * resizer;
#endif

	int savedStrut;

	int indexToLastNonPermanentWidget() const
	{
		int i = items.size() - 1;
		for (; i >= 0; --i) {
			SBItem *item = items.at(i);
			if (!(item && item->p))
				break;
		}
		return i;
	}
};

SimpleStatusBar::SimpleStatusBar(QWidget *parent)
  : QStatusBar(parent), d_ptr(new SimpleStatusBarPrivate)
{
}

SimpleStatusBar::SimpleStatusBar(SimpleStatusBarPrivate &dd, QWidget *parent)
  : QStatusBar(parent), d_ptr(&dd)
{
}

void SimpleStatusBar::paintEvent(QPaintEvent *)
{
  Q_D(SimpleStatusBar);
	bool haveMessage = !d->tempItem.isEmpty();

	QPainter p(this);
  SimpleStatusBarPrivate::SBItem* item = 0;

	bool rtl = layoutDirection() == Qt::RightToLeft;

	int left = 6;
	int right = width() - 12;

  /*
#ifndef QT_NO_SIZEGRIP
 if (d->resizer && d->resizer->isVisible()) {
  if (rtl)
    left = d->resizer->x() + d->resizer->width();
  else
    right = d->resizer->x();
 }
#endif
*/
	for (int i=0; i<d->items.size(); ++i) {
		item = d->items.at(i);
		if (!item)
			break;
		if (!haveMessage || item->p)
			if (item->w->isVisible()) {
				if (item->p) {
					if (rtl)
						left = qMax(left, item->w->x() + item->w->width() + 2);
					else
						right = qMin(right, item->w->x()-1);
				}
				QStyleOption opt(0);
				opt.rect.setRect(item->w->x() - 2, item->w->y() - 1,
                         item->w->width() + 4, item->w->height() + 2);
				opt.palette = palette();
				opt.state = QStyle::State_None;
        style()->drawPrimitive(QStyle::PE_FrameStatusBarItem, &opt, &p, item->w);
			}
	}
	if (haveMessage) {
    p.setPen(palette().windowText().color());
		p.drawText(left, 0, right-left, height(), Qt::AlignLeading | Qt::AlignVCenter | Qt::TextSingleLine, d->tempItem);
	}
}

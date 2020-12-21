/*
  Source code extracted from:
  http://www.jakepetroules.com/2011/07/10/creating-a-windows-explorer-style-search-box-in-qt

  Written by Jake Petroules
  Adapted to suit QTGZManager
*/

#include "searchlineedit.h"
#include "wmhelper.h"

#include <QToolButton>
#include <QStyle>

SearchLineEdit::SearchLineEdit(QWidget *parent) :
  QLineEdit(parent){

  // Create the search button and set its icon, cursor, and stylesheet
  this->m_SearchButton = new QToolButton(this);
  //this->mSearchButton->setFixedSize(18, 18);

  if (WMHelper::isKDERunning())
    this->m_SearchButton->setFixedSize(18, 18);
  else
    this->m_SearchButton->setFixedSize(16, 16);

  this->m_SearchButton->setCursor(Qt::ArrowCursor);
  this->m_SearchButton->setStyleSheet(this->buttonStyleSheetForCurrentState());

  // Update the search button when the text changes
  QObject::connect(this, SIGNAL(textChanged(QString)), SLOT(updateSearchButton(QString)));

  // Some stylesheet and size corrections for the text box
#if QT_VERSION >= 0x040700
  this->setPlaceholderText(QObject::tr("Find"));
#else
  this->setToolTip(QObject::tr("Find"));
#endif

  this->setStyleSheet(this->styleSheetForCurrentState());
}

void SearchLineEdit::resizeEvent(QResizeEvent *event)
{
  Q_UNUSED(event);
  //QSize size = this->m_SearchButton->sizeHint();
  //this->m_SearchButton->move(5, (this->rect().bottom() + 10 - size.height()) / 2);
  this->m_SearchButton->move(5, (this->rect().height() - this->m_SearchButton->height()) / 2);
}

void SearchLineEdit::updateSearchButton(const QString &text)
{
  if (!text.isEmpty()){
    // We have some text in the box - set the button to clear the text
    QObject::connect(this->m_SearchButton, SIGNAL(clicked()), SLOT(clear()));
  }
  else{
    // The text box is empty - make the icon do nothing when clicked
    QObject::disconnect(this->m_SearchButton, SIGNAL(clicked()), this, SLOT(clear()));
  }

  //this->setStyleSheet(this->styleSheetForCurrentState());
  this->m_SearchButton->setStyleSheet(this->buttonStyleSheetForCurrentState());
}

QString SearchLineEdit::styleSheetForCurrentState() const
{
  int frameWidth = 1; //this->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
  QString style;
  style += "QLineEdit {";

  if (this->text().isEmpty()){
    style += "font-family: 'Sans Serif';";
    style += "font-style: italic;";
  }

  if (!WMHelper::isKDERunning()) //UnixCommand::getLinuxDistro() != ectn_CHAKRA)
  {
    int frameWidth = 1;
    style += QLatin1String("padding-left: 20px;");
    style += QStringLiteral("padding-right: %1px;").arg(this->m_SearchButton->sizeHint().width() + frameWidth + 1);
    style += QLatin1String("border-width: 3px;}");
    //style += "border-image: url(:/resources/images/esf-border.png) 3 3 3 3 stretch;}";
    //style += "background-color: rgba(255, 255, 255, 255);"; //204);";
    //style += "color: black;}";
  }
  else
  {
    style += QLatin1String("padding-left: 20px;}");
    //setPalette(QApplication::palette());
  }

  /*style += "padding-left: 20px;";
  style += QString("padding-right: %1px;").arg(this->m_SearchButton->sizeHint().width() + frameWidth + 1);
  style += "border-width: 3px;";
  style += "border-image: url(:/resources/images/esf-border.png) 3 3 3 3 stretch;";
  style += "background-color: rgba(255, 255, 255, 255);"; //204);";
  style += "color: black;}";
  //style += "QLineEdit:hover, QLineEdit:focus {";
  //style += "background-color: rgba(255, 255, 255, 255);";
  //style += "}";*/

  return style;
}

void SearchLineEdit::setFoundStyle(){
  QString style = "QLineEdit {";
  style += "font-family: 'Sans Serif';";
  style += "font-style: italic;";
  style += "padding-left: 20px;";
  style += QString("padding-right: %1px;").arg(this->m_SearchButton->sizeHint().width() + 2);
  style += "border-width: 3px;}";
  /*style += "border-image: url(:/resources/images/esf-border.png) 3 3 3 3 stretch;";
  style += "color: black; ";
  style += "background-color: rgb(255, 255, 200);";
  style += "border-color: rgb(206, 204, 197);}";*/

  setStyleSheet(style);
}

void SearchLineEdit::setNotFoundStyle(){
  QString style = "QLineEdit {";
  style += "font-family: 'Sans Serif';";
  style += "font-style: italic;";
  style += "padding-left: 20px;";
  style += QString("padding-right: %1px;").arg(this->m_SearchButton->sizeHint().width() + 2);
  style += "border-width: 3px;";
  style += "border-image: url(:/resources/images/esf-border.png) 3 3 3 3 stretch;";
  style += "color: black; ";
  style += "background-color: lightgray;"; //palette(mid);"; //rgb(207, 135, 142);";
  style += "border-color: rgb(206, 204, 197);}";

  setStyleSheet(style);
}

QString SearchLineEdit::buttonStyleSheetForCurrentState() const
{
  QString style;
  style += "QToolButton {";
  style += "border: none; margin: 0; padding: 0;";
  style += QString("background-image: url(:/resources/images/esf-%1.png);").arg(this->text().isEmpty() ? "search" : "clear");
  style += "}";

  if (!this->text().isEmpty()){
    style += "QToolButton:hover { background-image: url(:/resources/images/esf-clear-hover.png); }";
    style += "QToolButton:pressed { background-image: url(:/resources/images/esf-clear-active.png); }";
    this->m_SearchButton->setToolTip(tr("Clear"));
  }
  else this->m_SearchButton->setToolTip("");

  return style;
}

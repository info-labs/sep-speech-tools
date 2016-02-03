#ifndef SPEECHOPTIMIZER_H
#define SPEECHOPTIMIZER_H

#include "mecabnode.h"

class SpeechOptimizer
{
public:
    SpeechOptimizer(MeCab::Tagger *tagger);

    double calcSpeechCount(const MeCabResult &nodes);
    double calcSpeechCount(const MeCabNode &node);

    QString toRubyHtml(const MeCabResult &nodes);
    QString toRubyHtml(const MeCabNode &node);

    QString toSpeech(const MeCabResult &nodes);
    QString toSpeech(const MeCabNode &node);

private:
    double calcSpeechCount(QString speech);

    void heuristicInitialize() const;
    QString heuristicSpeech(const QString &surface) const;

    QString numToKanji(const QString &numstr) const;
    MeCabResult parse(const QString &data);

    MeCab::Tagger *tagger;

    const QString KIGOU;
    const QString TOUTEN;
    const QString KUTEN;
    const QString PERCENT;

    QRegExp katakana;
    QRegExp youon_kigou;
    QRegExp sokuon;

    QRegExp ascii;

    QRegExp nums;
    QString numKanji;
    QString numPlace1;
    QString numPlace2;

};

#endif // SPEECHOPTIMIZER_H

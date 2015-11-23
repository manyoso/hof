#ifndef cache_h
#define cache_h

#include "combinators.h"

#include <QtCore>

class EvaluationCache {
public:
    static EvaluationCache* instance()
    {
        static EvaluationCache* s_instance = 0;
        if (!s_instance)
            s_instance = new EvaluationCache;
        return s_instance;
    }

    void insert(const QString& key, const CombinatorPtr& value);
    CombinatorPtr result(const QString& key) const;

private:
    EvaluationCache()
    { }

    QHash<QString, CombinatorPtr> m_cache;
};

#endif // cache_h

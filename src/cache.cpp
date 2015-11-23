#include "cache.h"

void EvaluationCache::insert(const QString& key, const CombinatorPtr& value)
{
    if (m_cache.contains(key) || key == value->toString())
        return;

    CombinatorPtr v = value;
    while (m_cache.contains(v->toString()))
        v = m_cache.value(v->toString());

    if (!m_cache.contains(key))
        m_cache.insert(key, v);
}

CombinatorPtr EvaluationCache::result(const QString& key) const
{
    return m_cache.value(key, CombinatorPtr());
}

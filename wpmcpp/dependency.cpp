#include "dependency.h"
#include "repository.h"
#include "packageversion.h"
#include "package.h"

Dependency::Dependency()
{
    this->minIncluded = true;
    this->min.setVersion(0, 0);
    this->maxIncluded = false;
    this->max.setVersion(1, 0);
}

QString Dependency::toString()
{
    QString res;

    Repository* r = Repository::getDefault();
    Package* p = r->findPackage(this->package);
    if (p)
        res.append(p->title);
    else
        res.append(package);

    res.append(" ");

    if (minIncluded)
        res.append('[');
    else
        res.append('(');

    res.append(this->min.getVersionString());

    res.append(", ");

    res.append(this->max.getVersionString());

    if (maxIncluded)
        res.append(']');
    else
        res.append(')');

    return res;
}

bool Dependency::isInstalled()
{
    Repository* r = Repository::getDefault();
    QList<PackageVersion*> installed = r->getInstalled();
    bool res = false;
    for (int i = 0; i < installed.count(); i++) {
        PackageVersion* pv = installed.at(i);
        if (pv->package == this->package && pv->installed() &&
                this->test(pv->version)) {
            res = true;
            break;
        }
    }
    return res;
}

void Dependency::findAllInstalledMatches(QList<PackageVersion*>& res)
{
    Repository* r = Repository::getDefault();
    QList<PackageVersion*> installed = r->getInstalled();
    for (int i = 0; i < installed.count(); i++) {
        PackageVersion* pv = installed.at(i);
        if (pv->package == this->package && this->test(pv->version)) {
            res.append(pv);
        }
    }
}

bool Dependency::setVersions(const QString versions)
{
    QString versions_ = versions;

    bool minIncluded_, maxIncluded_;

    // qDebug() << "Repository::createDependency.1" << versions;

    if (versions_.startsWith('['))
        minIncluded_ = true;
    else if (versions_.startsWith('('))
        minIncluded_ = false;
    else
        return false;
    versions_.remove(0, 1);

    // qDebug() << "Repository::createDependency.1.1" << versions;

    if (versions_.endsWith(']'))
        maxIncluded_ = true;
    else if (versions_.endsWith(')'))
        maxIncluded_ = false;
    else
        return false;
    versions_.chop(1);

    // qDebug() << "Repository::createDependency.2";

    QStringList parts = versions_.split(',');
    if (parts.count() != 2)
        return false;

    Version min_, max_;
    if (!min_.setVersion(parts.at(0).trimmed()) ||
            !max_.setVersion(parts.at(1).trimmed()))
        return false;
    this->minIncluded = minIncluded_;
    this->min = min_;
    this->maxIncluded = maxIncluded_;
    this->max = max_;

    return true;
}

PackageVersion* Dependency::findBestMatchToInstall()
{
    Repository* r = Repository::getDefault();
    PackageVersion* res = 0;
    for (int i = 0; i < r->packageVersions.count(); i++) {
        PackageVersion* pv = r->packageVersions.at(i);
        if (pv->package == this->package && this->test(pv->version) &&
                !pv->external) {
            if (res == 0 || pv->version.compare(res->version) > 0)
                res = pv;
        }
    }
    return res;
}

PackageVersion* Dependency::findHighestInstalledMatch()
{
    QList<PackageVersion*> list;
    findAllInstalledMatches(list);
    PackageVersion* res = 0;
    for (int i = 0; i < list.count(); i++) {
        PackageVersion* pv = list.at(i);
        if (res == 0 || pv->version.compare(res->version) > 0)
            res = pv;
    }
    return res;
}

bool Dependency::test(const Version& v)
{
    int a = v.compare(this->min);
    int b = v.compare(this->max);

    bool low;
    if (minIncluded)
        low = a >= 0;
    else
        low = a > 0;

    bool high;
    if (maxIncluded)
        high = b <= 0;
    else
        high = b < 0;

    return low && high;
}


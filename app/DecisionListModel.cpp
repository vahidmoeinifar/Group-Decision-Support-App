//////////////////////////////////////////////////
//             BY: Vahid Moeinifar              //
//   PhD student of AGH university of Krakow    //
//                                              //
//////////////////////////////////////////////////

#include "DecisionListModel.h"

#include "DecisionService.h"

DecisionListModel::DecisionListModel(DecisionService& service, QObject* parent)
    : QAbstractListModel(parent), service_(service)
{
}

int DecisionListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : count();
}

QVariant DecisionListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= items_.size())
        return {};

    const DecisionSummary& d = items_.at(index.row());
    switch (role) {
    case IdRole: return d.id;
    case TitleRole: return d.title;
    case RoundRole: return d.currentRound;
    case IsOwnerRole: return d.isOwner;
    case RoleTextRole: {
        const QString role = d.isOwner && d.isExpert ? tr("Organizer and expert")
                           : d.isOwner              ? tr("Organizer")
                                                    : tr("Expert");
        return tr("%1, round %2").arg(role).arg(d.currentRound);
    }
    default: return {};
    }
}

QHash<int, QByteArray> DecisionListModel::roleNames() const
{
    return {{IdRole, "decisionId"}, {TitleRole, "title"}, {RoleTextRole, "roleText"},
            {RoundRole, "round"}, {IsOwnerRole, "isOwner"}};
}

void DecisionListModel::refresh(int userId)
{
    const int before = count();
    beginResetModel();
    items_ = userId > 0 ? service_.list(userId) : QList<DecisionSummary>{};
    endResetModel();
    if (before != count())
        emit countChanged();
}

int DecisionListModel::idAt(int row) const
{
    return row >= 0 && row < items_.size() ? items_.at(row).id : -1;
}

int DecisionListModel::rowOf(int decisionId) const
{
    for (int i = 0; i < items_.size(); ++i)
        if (items_.at(i).id == decisionId)
            return i;
    return -1;
}

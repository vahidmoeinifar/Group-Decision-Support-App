//////////////////////////////////////////////////
//             BY: Vahid Moeinifar              //
//   PhD student of AGH university of Krakow    //
//                                              //
//////////////////////////////////////////////////

#pragma once

#include "Domain.h"

#include <QAbstractListModel>
#include <QtQml/qqmlregistration.h>

class DecisionService;

/// The decisions in the sidebar: the ones the user organizes or is an expert in.
class DecisionListModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("DecisionListModel is created by AppController.")

    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Role { IdRole = Qt::UserRole + 1, TitleRole, RoleTextRole, RoundRole, IsOwnerRole };

    DecisionListModel(DecisionService& service, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return static_cast<int>(items_.size()); }

    /// Reloads the list for a user (0 clears it).
    void refresh(int userId);
    Q_INVOKABLE int idAt(int row) const;
    Q_INVOKABLE int rowOf(int decisionId) const;

signals:
    void countChanged();

private:
    DecisionService& service_;
    QList<DecisionSummary> items_;
};

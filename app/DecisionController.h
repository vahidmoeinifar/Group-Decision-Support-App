//////////////////////////////////////////////////
//             BY: Vahid Moeinifar              //
//   PhD student of AGH university of Krakow    //
//                                              //
//////////////////////////////////////////////////

#pragma once

#include "Domain.h"

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

#include <optional>

class DecisionService;
class Session;

/// The decision that is open on screen. Turns service results into simple
/// QVariant maps and lists that QML can bind to directly.
/// Methods return { ok: false, error: "..." } instead of throwing.
class DecisionController : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("DecisionController is created by AppController.")

    Q_PROPERTY(bool loaded READ loaded NOTIFY changed)
    Q_PROPERTY(int decisionId READ decisionId NOTIFY changed)
    Q_PROPERTY(QString title READ title NOTIFY changed)
    Q_PROPERTY(QString description READ description NOTIFY changed)
    Q_PROPERTY(QString ownerName READ ownerName NOTIFY changed)
    Q_PROPERTY(QString createdText READ createdText NOTIFY changed)
    Q_PROPERTY(bool isOwner READ isOwner NOTIFY changed)
    Q_PROPERTY(int myExpertId READ myExpertId NOTIFY changed)
    Q_PROPERTY(int currentRound READ currentRound NOTIFY changed)
    Q_PROPERTY(double consensusTarget READ consensusTarget NOTIFY changed)
    Q_PROPERTY(int completeCount READ completeCount NOTIFY changed)
    Q_PROPERTY(QVariantList options READ options NOTIFY changed)
    Q_PROPERTY(QVariantList criteria READ criteria NOTIFY changed)
    Q_PROPERTY(QVariantList experts READ experts NOTIFY changed)
    /// Goes up on every reload; bind to it to recompute results when anything changes.
    Q_PROPERTY(int revision READ revision NOTIFY changed)

public:
    DecisionController(DecisionService& service, Session& session, QObject* parent = nullptr);

    bool loaded() const { return decision_.has_value(); }
    int decisionId() const { return decision_ ? decision_->id : -1; }
    QString title() const { return decision_ ? decision_->title : QString(); }
    QString description() const { return decision_ ? decision_->description : QString(); }
    QString ownerName() const { return decision_ ? decision_->ownerName : QString(); }
    QString createdText() const;
    bool isOwner() const;
    int myExpertId() const;
    int currentRound() const { return decision_ ? decision_->currentRound : 0; }
    double consensusTarget() const { return decision_ ? decision_->consensusTarget : 0.0; }
    int completeCount() const;
    QVariantList options() const;
    QVariantList criteria() const;
    QVariantList experts() const;
    int revision() const { return revision_; }

    Q_INVOKABLE void open(int decisionId);
    Q_INVOKABLE void close();

    /// method: "Topsis", "WeightedSum" or "Borda"
    Q_INVOKABLE QVariantMap result(const QString& method) const;
    Q_INVOKABLE QVariantMap compare() const;
    Q_INVOKABLE QVariantList history() const;
    Q_INVOKABLE QVariantMap sensitivity(const QString& method) const;
    Q_INVOKABLE QVariantMap scores(int expertId) const;
    Q_INVOKABLE QVariantMap feedback(int expertId) const;

    /// cells: { "optionId:criterionId": number }
    Q_INVOKABLE bool saveScores(int expertId, const QVariantMap& cells);
    Q_INVOKABLE bool startNextRound();
    Q_INVOKABLE bool removeDecision();

    /// form: { title, description, target, options: [..], criteria: [{name, weight, isCost}],
    ///         experts: [{name, email, weight}], addMe: bool }. Returns the new id or -1.
    Q_INVOKABLE int create(const QVariantMap& form);

    Q_INVOKABLE static QString cellKey(int optionId, int criterionId);

    /// Called when another window changed the database. Reloads and tells the user what changed.
    void checkForOutsideChange();

signals:
    void changed();
    void closed();
    /// kind: "info", "success" or "error"
    void notify(const QString& message, const QString& kind);
    /// The list of decisions should be reloaded (created, deleted or round changed).
    void listChanged();

private:
    void reload();
    static std::optional<gd::Method> parseMethod(const QString& method);
    QVariantMap failure(const QString& message) const;

    DecisionService& service_;
    Session& session_;
    std::optional<Decision> decision_;
    int revision_ = 0;
};

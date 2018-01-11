/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

struct HistoryServiceDependentData {
	MsgId msgId = 0;
	HistoryItem *msg = nullptr;
	ClickHandlerPtr lnk;
};

struct HistoryServicePinned
	: public RuntimeComponent<HistoryServicePinned>
	, public HistoryServiceDependentData {
};

struct HistoryServiceGameScore
	: public RuntimeComponent<HistoryServiceGameScore>
	, public HistoryServiceDependentData {
	int score = 0;
};

struct HistoryServicePayment
	: public RuntimeComponent<HistoryServicePayment>
	, public HistoryServiceDependentData {
	QString amount;
};

struct HistoryServiceSelfDestruct
	: public RuntimeComponent<HistoryServiceSelfDestruct> {
	enum class Type {
		Photo,
		Video,
	};
	Type type = Type::Photo;
	TimeMs timeToLive = 0;
	TimeMs destructAt = 0;
};

namespace HistoryView {
class ServiceMessagePainter;
} // namespace HistoryView

class HistoryService : public HistoryItem, private HistoryItemInstantiated<HistoryService> {
public:
	struct PreparedText {
		QString text;
		QList<ClickHandlerPtr> links;
	};

	static not_null<HistoryService*> create(not_null<History*> history, const MTPDmessage &message) {
		return _create(history, message);
	}
	static not_null<HistoryService*> create(not_null<History*> history, const MTPDmessageService &message) {
		return _create(history, message);
	}
	static not_null<HistoryService*> create(not_null<History*> history, MsgId msgId, QDateTime date, const PreparedText &message, MTPDmessage::Flags flags = 0, UserId from = 0, PhotoData *photo = nullptr) {
		return _create(history, msgId, date, message, flags, from, photo);
	}

	bool updateDependencyItem() override;
	MsgId dependencyMsgId() const override {
		if (auto dependent = GetDependentData()) {
			return dependent->msgId;
		}
		return 0;
	}
	bool notificationReady() const override {
		if (auto dependent = GetDependentData()) {
			return (dependent->msg || !dependent->msgId);
		}
		return true;
	}

	QRect countGeometry() const;

	void draw(Painter &p, QRect clip, TextSelection selection, TimeMs ms) const override;
	bool hasPoint(QPoint point) const override;
	HistoryTextState getState(QPoint point, HistoryStateRequest request) const override;

	[[nodiscard]] TextSelection adjustSelection(
			TextSelection selection,
			TextSelectType type) const override {
		return _text.adjustSelection(selection, type);
	}

	void applyEdition(const MTPDmessageService &message) override;
	TimeMs getSelfDestructIn(TimeMs now) override;

	Storage::SharedMediaTypesMask sharedMediaTypes() const override;

	bool needCheck() const override {
		return false;
	}
	bool serviceMsg() const override {
		return true;
	}
	TextWithEntities selectedText(TextSelection selection) const override;
	QString inDialogsText(DrawInDialog way) const override;
	QString inReplyText() const override;

	~HistoryService();

protected:
	friend class HistoryView::ServiceMessagePainter;

	HistoryService(not_null<History*> history, const MTPDmessage &message);
	HistoryService(not_null<History*> history, const MTPDmessageService &message);
	HistoryService(not_null<History*> history, MsgId msgId, QDateTime date, const PreparedText &message, MTPDmessage::Flags flags = 0, UserId from = 0, PhotoData *photo = 0);
	friend class HistoryItemInstantiated<HistoryService>;

	void initDimensions() override;
	int resizeContentGetHeight() override;

	void markMediaAsReadHook() override;

	void setServiceText(const PreparedText &prepared);

	QString fromLinkText() const {
		return textcmdLink(1, _from->name);
	};
	ClickHandlerPtr fromLink() const {
		return _from->createOpenLink();
	};

	void removeMedia();

private:
	HistoryServiceDependentData *GetDependentData() {
		if (auto pinned = Get<HistoryServicePinned>()) {
			return pinned;
		} else if (auto gamescore = Get<HistoryServiceGameScore>()) {
			return gamescore;
		} else if (auto payment = Get<HistoryServicePayment>()) {
			return payment;
		}
		return nullptr;
	}
	const HistoryServiceDependentData *GetDependentData() const {
		return const_cast<HistoryService*>(this)->GetDependentData();
	}
	bool updateDependent(bool force = false);
	void updateDependentText();
	void clearDependency();

	void createFromMtp(const MTPDmessage &message);
	void createFromMtp(const MTPDmessageService &message);
	void setMessageByAction(const MTPmessageAction &action);
	void setSelfDestruct(HistoryServiceSelfDestruct::Type type, int ttlSeconds);

	PreparedText preparePinnedText();
	PreparedText prepareGameScoreText();
	PreparedText preparePaymentSentText();

};

class HistoryJoined : public HistoryService, private HistoryItemInstantiated<HistoryJoined> {
public:
	static not_null<HistoryJoined*> create(not_null<History*> history, const QDateTime &inviteDate, not_null<UserData*> inviter, MTPDmessage::Flags flags) {
		return _create(history, inviteDate, inviter, flags);
	}

protected:
	HistoryJoined(not_null<History*> history, const QDateTime &inviteDate, not_null<UserData*> inviter, MTPDmessage::Flags flags);
	using HistoryItemInstantiated<HistoryJoined>::_create;
	friend class HistoryItemInstantiated<HistoryJoined>;

private:
	static PreparedText GenerateText(not_null<History*> history, not_null<UserData*> inviter);

};

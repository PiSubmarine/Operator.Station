#pragma once

#include <QObject>
#include <QString>

namespace PiSubmarine::Operator::Station::Input::View
{
    class BindingEntryViewModel final : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QString name READ GetName CONSTANT)
        Q_PROPERTY(QString keyboardHint READ GetKeyboardHint NOTIFY KeyboardHintChanged)
        Q_PROPERTY(QString gamepadHint READ GetGamepadHint NOTIFY GamepadHintChanged)
        Q_PROPERTY(bool keyboardCapturing READ GetKeyboardCapturing NOTIFY KeyboardCapturingChanged)
        Q_PROPERTY(bool gamepadCapturing READ GetGamepadCapturing NOTIFY GamepadCapturingChanged)

    public:
        explicit BindingEntryViewModel(QString name, QObject* parent = nullptr);

        [[nodiscard]] QString GetName() const;
        [[nodiscard]] QString GetKeyboardHint() const;
        [[nodiscard]] QString GetGamepadHint() const;
        [[nodiscard]] bool GetKeyboardCapturing() const;
        [[nodiscard]] bool GetGamepadCapturing() const;

    public slots:
        void SetKeyboardHint(const QString& hint);
        void SetGamepadHint(const QString& hint);
        void SetKeyboardCapturing(bool capturing);
        void SetGamepadCapturing(bool capturing);

    signals:
        void KeyboardHintChanged();
        void GamepadHintChanged();
        void KeyboardCapturingChanged();
        void GamepadCapturingChanged();

    private:
        QString m_Name;
        QString m_KeyboardHint = "Unbound";
        QString m_GamepadHint = "Unbound";
        bool m_KeyboardCapturing = false;
        bool m_GamepadCapturing = false;
    };
}

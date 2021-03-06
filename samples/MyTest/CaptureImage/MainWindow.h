#pragma once
class MainWindow : public ui::WindowImplBase
{
public:
	MainWindow();
	~MainWindow();

public:
	virtual std::wstring GetSkinFolder() override;
	virtual std::wstring GetSkinFile() override;
	virtual std::wstring GetWindowClassName() const override;

public:
	virtual void InitWindow() override;

	virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	static const std::wstring kClassName;


public:
	void SetDisplayRect(int n,bool bPrimary,RECT rect);
	RECT _rect;
	bool _bPrimary = false;
	int _nScreenNum = 0;

	std::shared_ptr<ui::ImageInfo> pInfo;
};


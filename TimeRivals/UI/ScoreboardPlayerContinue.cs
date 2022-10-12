using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.InputSystem;
using UnityEngine.InputSystem.UI;


public class ScoreboardPlayerContinue : MonoBehaviour
{
    [SerializeField] private GameObject _mainMenuButton;
    [SerializeField] private Button _thisButton;
    [SerializeField] private Color pressedColor;

    private bool _hasClicked;

    [SerializeField] private int _playerID;
    public int PlayerID { get { return _playerID; } }

    public void ConfirmedClick()
    {
        if (_hasClicked)
        {
            return;
        }
        _hasClicked = true;

        ColorBlock cb = _thisButton.colors;
        cb.normalColor = pressedColor;
        cb.highlightedColor = pressedColor;
        cb.pressedColor = pressedColor;
        cb.selectedColor = pressedColor;
        cb.disabledColor = pressedColor;
        _thisButton.colors = cb;

        _mainMenuButton.GetComponent<ChangeSceneButton>().UpdateScoreboardMainMenuButtonCount();
    }
}

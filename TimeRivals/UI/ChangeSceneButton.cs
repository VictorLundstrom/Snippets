using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.InputSystem;
using UnityEngine.InputSystem.UI;
using UnityEngine.EventSystems;

using UnityEngine.SceneManagement;
using TMPro;


public class ChangeSceneButton : MonoBehaviour
{
    [SerializeField] private GameObject _firstSelectedButton;
    [SerializeField] private bool _usedInScoreboard;
    private TextMeshProUGUI _buttonText;

    private int _requiredButtonPresses;
    private int _currButtonPresses;

    public void Start()
    {
        if (_usedInScoreboard)
        {
            _requiredButtonPresses = PlayerSetup.instance.PlayerList.Count;

            _buttonText = GetComponentInChildren<TextMeshProUGUI>();
            _buttonText.text += " " + _currButtonPresses + "/" + _requiredButtonPresses; //Append button text with required button presses
        }
    }

    private void OnEnable()
    {
        if (gameObject.scene == SceneManager.GetSceneByBuildIndex((int)SceneIndexes.MANAGER)) //If this button is in Persistent scene, AKA Pause Menu
            return;

        PlayerSetup.instance.SwitchActionMapForAll("UI");

        if (gameObject.scene == SceneManager.GetSceneByBuildIndex((int)SceneIndexes.SCOREBOARD))
        {
            return;
        }

        EventSystem eventSystem = GameObject.FindGameObjectWithTag("EventSystem").GetComponent<EventSystem>();
        eventSystem.SetSelectedGameObject(null);
        eventSystem.SetSelectedGameObject(_firstSelectedButton);

    }

    //Called by pressing In-Game buttons
    public void ContinueOrEndGame()
    {
        PlayerSetup.instance.SwitchActionMapForAll("Player");

        PlayerSetup.instance.ResetPickupIndicator(); //Used to Disable PickupIndicator to prevent error
        PlayerSetup.instance.ResetElectrocution(); //Sometimes this VFX/animation gets through scenes, so stop it if its active

        if (LevelManager.Instance.CurrLevel < LevelManager.Instance.NumLevels) //If we should go to Shop
        {
            LevelManager.Instance.LoadShop();
        }
        else //If we should go to Scoreboard
        {
            Destroy(GameObject.FindGameObjectWithTag("EventSystem"));
            LevelManager.Instance.LoadScoreboard();
        }
    }

    //Called by pressing Menu buttons
    public void ReturnToMainMenu()
    {
        Time.timeScale = 1; //Set to 1 if we where in pause menu

        DestroyEverything(); //Destroy all, including "Don't Destroy On Load"-Gameobjects, such as Players

        LevelManager.Instance.LoadMainMenu();
    }

    public void StartGame()
    {
        LevelManager.Instance.StartGame(); //Load first level in array of shuffled levels
    }

    public void LeaveShop()
    {
        LevelManager.Instance.LoadNextLevel();
    }

    public void UpdateScoreboardMainMenuButtonCount()
    {
        string currbuttonPress = _currButtonPresses.ToString();
        string currbuttonPressPlusOne = (_currButtonPresses + 1).ToString();
        _buttonText.text = _buttonText.text.Replace(currbuttonPress, currbuttonPressPlusOne);
        _currButtonPresses++;

        if (_currButtonPresses == _requiredButtonPresses)
        {
            ReturnToMainMenu();
        }
    }
    private void DestroyEverything()
    {
        GameObject[] GameObjects = FindObjectsOfType<GameObject>();

        for (int i = 0; i < GameObjects.Length; i++)
        {
            Destroy(GameObjects[i]);
        }
    }

    public void QuitGamePC()
    {
        Application.Quit();
    }
}

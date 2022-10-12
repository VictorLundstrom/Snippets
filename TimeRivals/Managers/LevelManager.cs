using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;
using TMPro;
using UnityEngine.InputSystem;

public class LevelManager : MonoBehaviour
{
    public static LevelManager Instance;

    public GameObject LoadingScreen;
    public Image LoadingBar;
    public TextMeshProUGUI LoadingText;

    //DontDestroyOnLoad System
    [SerializeField] private GameObject _system;

    //Level Setup
    [SerializeField] private int _numLevels; //Needs same amount of Levels as in Build settings and SceneIndexes
    public int NumLevels { get { return _numLevels; } }
    private int[] _levelsArr;
    //Curr LevelIndex
    private int _currLevel = 0;
    public int CurrLevel { get { return _currLevel + 1; } }

    //Loading Data
    List<AsyncOperation> scenesLoading = new List<AsyncOperation>();
    private float totalSceneProgress;

    private int _playMode = (int)PlayModes.UNSET;
    public int PlayMode { get { return _playMode; } }


    private void Awake()
    {
        Instance = this;

        _levelsArr = new int[_numLevels];

        scenesLoading.Add(SceneManager.LoadSceneAsync((int)SceneIndexes.MAIN_MENU, LoadSceneMode.Additive)); //First scene to load

        StartCoroutine(GetSceneLoadProgress((int)SceneIndexes.MAIN_MENU));
    }

    public void StartGame()
    {
        _currLevel = 0; //Reset back to 0 if we played again, should be put in a Resetvariables-function instead

        PlayerInputManager.instance.DisableJoining();

        ChoosePlayMode(); //Choose Gamemode depending on amount of players

        FillLevelArray(_levelsArr);
        ShuffleLevels(_levelsArr);

        LoadingScreen.SetActive(true);

        scenesLoading.Add(SceneManager.UnloadSceneAsync((int)SceneIndexes.MAIN_MENU));
        scenesLoading.Add(SceneManager.LoadSceneAsync(_levelsArr[_currLevel], LoadSceneMode.Additive)); //Load first level

        StartCoroutine(GetSceneLoadProgress(_levelsArr[_currLevel], true));

    }

    public void LoadShop()
    {
        List<GameObject> playerList = PlayerSetup.instance.PlayerList;

        LoadingScreen.SetActive(true);

        scenesLoading.Add(SceneManager.UnloadSceneAsync(_levelsArr[_currLevel]));
        scenesLoading.Add(SceneManager.LoadSceneAsync((int)SceneIndexes.SHOP, LoadSceneMode.Additive)); //Load Shop

        _currLevel++;

        StartCoroutine(GetSceneLoadProgress((int)SceneIndexes.SHOP));

        // Resets the poweups
        for (int i = 0; i < playerList.Count; i++) 
            playerList[i].GetComponent<PlayerController>().ResetPowerups();
    }

    public void LoadNextLevel()
    {
        LoadingScreen.SetActive(true);

        scenesLoading.Add(SceneManager.UnloadSceneAsync((int)SceneIndexes.SHOP));
        scenesLoading.Add(SceneManager.LoadSceneAsync(_levelsArr[_currLevel], LoadSceneMode.Additive)); //Load Next level

        StartCoroutine(GetSceneLoadProgress(_levelsArr[_currLevel]));
    }

    public void LoadScoreboard()
    {
        LoadingScreen.SetActive(true);

        scenesLoading.Add(SceneManager.UnloadSceneAsync(_levelsArr[_currLevel]));
        scenesLoading.Add(SceneManager.LoadSceneAsync((int)SceneIndexes.SCOREBOARD, LoadSceneMode.Additive)); //Load Scoreboard

        EmptyLevelArray(_levelsArr);

        StartCoroutine(GetSceneLoadProgress((int)SceneIndexes.SCOREBOARD));
    }

    public void LoadMainMenu()
    {
        LoadingScreen.SetActive(true);

        SceneManager.LoadScene((int)SceneIndexes.MANAGER); //Load Persistant scene
    }

    //////////////////////////////////Internal Functions//////////////////////////////////
    public IEnumerator GetSceneLoadProgress(int nextLevelIndex, bool firstLevel = false)
    {
        for (int i = 0; i < scenesLoading.Count; i++)
        {
            while (!scenesLoading[i].isDone)
            {
                totalSceneProgress = 0;

                foreach (AsyncOperation operation in scenesLoading)
                {
                    totalSceneProgress += operation.progress;
                }

                totalSceneProgress = (totalSceneProgress / scenesLoading.Count) * 100.0f; //returns Percentage in decimal form, thats why we multiply by 100 to instead get %

                LoadingBar.fillAmount = Mathf.RoundToInt(totalSceneProgress);

                LoadingText.text = string.Format("{0}%", totalSceneProgress);

                yield return null;
            }
        }
        LoadingScreen.SetActive(false);
        SceneManager.SetActiveScene(SceneManager.GetSceneByBuildIndex(nextLevelIndex));

        if (firstLevel)
        {
            Instantiate(_system);
        }
    }

    private void ChoosePlayMode()
    {
        switch (PlayerSetup.instance.PlayerList.Count)
        {
            case 2:
                _playMode = (int)PlayModes.TWO_PLAYER;
                break;
            case 3:
                _playMode = (int)PlayModes.THREE_PLAYER;
                break;
            case 4:
                _playMode = (int)PlayModes.FOUR_PLAYER;
                break;
            default:
                Debug.Log("Invalid TotalPlayerCount at SetupPlayerData");
                break;
        }

    }
    void ShuffleLevels(int[] levelsArr)
    {
        for (int i = 0; i < levelsArr.Length - 1; i++)
        {
            int rnd = Random.Range(i, levelsArr.Length);

            // Simple swap
            int a = levelsArr[rnd];
            levelsArr[rnd] = levelsArr[i];
            levelsArr[i] = a;
        }
    }
    void FillLevelArray(int[] ar)
    {
        int levelSceneIndex = (int)SceneIndexes.LEVEL_0;

        for (int i = 0; i < ar.Length; i++)
        {
            ar[i] = levelSceneIndex;
            levelSceneIndex++;
        }
    }

    void EmptyLevelArray(int[] ar)
    {
        for (int i = 0; i < ar.Length; i++)
        {
            ar[i] = 0;
        }
    }
}
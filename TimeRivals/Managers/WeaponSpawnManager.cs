using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class WeaponSpawnManager : MonoBehaviour //There will be One instance of this script for each Stage.
{
    public static WeaponSpawnManager Instance;

    [SerializeField] private WeaponSpawnLocations _spawnManagerValues; //ScriptableObject, contains all spawnlocations
    [SerializeField] private GameObject[] _weaponPrefabs; //List of all weapon prefabs
    [SerializeField] private int _spawnIntervalMax; //How long until next Weapon spawns MAX
    [SerializeField] private int _spawnIntervalMin; //How long until next Weapon spawns MIN
    [SerializeField] private Timer _timer; //Reference to ingame timer
    [SerializeField] private Transform _mainCamera; //Reference to main camera to make Billboarding work

    [HideInInspector] public List<GameObject> ActivePickupList = new List<GameObject>();
    [HideInInspector] public List<Vector3> _pickedupList = new List<Vector3>();
    public List<Vector3> PickedupList { get { return _pickedupList; } }

    private float _timeSinceLastPickup;
    private int[] _spawnIntervals;
    private int _currInterval;

    private GameObject[] _randomizedWeaponPrefabs;
    private Vector3[] _randomizedSpawnLocations;
    private void Awake()
    {
        Instance = this;
        SetupRound();
    }

    private void Update()
    {
        if (_timer.RoundTimeShouldRun)
        {
            if (_timeSinceLastPickup > _spawnIntervals[_currInterval])
            {
                if (_pickedupList.Count < 4)
                {
                    SpawnWeapon(); //Also adds weapon to ActivePickupList with unique ID
                    if (_currInterval < _spawnIntervals.Length - 1)
                    {
                        _currInterval++;
                    }
                }

                _timeSinceLastPickup = 0;
            }
            _timeSinceLastPickup += Time.deltaTime;
        }
    }

    private void SpawnWeapon()
    {

        if (_pickedupList.Contains(_randomizedSpawnLocations[_currInterval]))
           return;

        GameObject weaponPickup = Instantiate(_randomizedWeaponPrefabs[_currInterval], _randomizedSpawnLocations[_currInterval], _randomizedWeaponPrefabs[_currInterval].transform.rotation); //Todo maybe don't need to set a rotation

        weaponPickup.name += _currInterval; //Append it's name by a unique number
        weaponPickup.GetComponent<SphereCollider>().enabled = true; //Enabled AFTER spawning because we want everything to be set up completly before checking triggers
        PickedupList.Add(weaponPickup.transform.position);
        ActivePickupList.Add(weaponPickup);
    }

    private void SetupRound()
    {
        int maxAmountOfPickups = (int)_timer.RoundTime / _spawnIntervalMin; //Highest potential amount of pickups based on our Roundtimer and our Min SpawnInterval values
        _spawnIntervals = new int[maxAmountOfPickups];
        _randomizedWeaponPrefabs = new GameObject[maxAmountOfPickups];
        _randomizedSpawnLocations = new Vector3[maxAmountOfPickups];
        for (int i = 0; i < _spawnIntervals.Length; i++)
        {
            _spawnIntervals[i] = Random.Range(_spawnIntervalMin, _spawnIntervalMax); //Set a random interval time between MIN and MAX values that were set in the Inspector.

            int randWeapon = Random.Range(0, _weaponPrefabs.Length); //Generate a Random weapon in the prefab list that was set in the Inspector.
            _randomizedWeaponPrefabs[i] = _weaponPrefabs[randWeapon];

            int randSpawnPos = Random.Range(0, _spawnManagerValues.SpawnPoints.Length); //Generate a Random spawn position from the Scriptable Object that was set in the Inspector.
            _randomizedSpawnLocations[i] = _spawnManagerValues.SpawnPoints[randSpawnPos];
        }
    }

}

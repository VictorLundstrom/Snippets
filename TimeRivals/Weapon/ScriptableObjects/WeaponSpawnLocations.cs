using UnityEngine;

[CreateAssetMenu(fileName = "WeaponSpawnLocations", menuName = "ScriptableObjects/WeaponSpawnLocations", order = 1)]
public class WeaponSpawnLocations : ScriptableObject
{
    [SerializeField] private Vector3 [] _spawnPoints;
    public Vector3 [] SpawnPoints { get { return _spawnPoints; } }
}
